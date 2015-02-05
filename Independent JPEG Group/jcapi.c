/* * jcapi.c * * Copyright (C) 1994, Thomas G. Lane. * This file is part of the Independent JPEG Group's software. * For conditions of distribution and use, see the accompanying README file. * * This file contains application interface code for the compression half of * the JPEG library.  Most of the routines intended to be called directly by * an application are in this file.  But also see jcparam.c for * parameter-setup helper routines, and jcomapi.c for routines shared by * compression and decompression. */#define JPEG_INTERNALS#include "jinclude.h"#include "jpeglib.h"/* * Initialization of a JPEG compression object. * The error manager must already be set up (in case memory manager fails). */GLOBAL voidjpeg_create_compress (j_compress_ptr cinfo){  int i;  /* For debugging purposes, zero the whole master structure.   * But error manager pointer is already there, so save and restore it.   */  {    struct jpeg_error_mgr * err = cinfo->err;    MEMZERO(cinfo, SIZEOF(struct jpeg_compress_struct));    cinfo->err = err;  }  cinfo->is_decompressor = FALSE;  /* Initialize a memory manager instance for this object */  jinit_memory_mgr((j_common_ptr) cinfo);  /* Zero out pointers to permanent structures. */  cinfo->progress = NULL;  cinfo->dest = NULL;  cinfo->comp_info = NULL;  for (i = 0; i < NUM_QUANT_TBLS; i++)    cinfo->quant_tbl_ptrs[i] = NULL;  for (i = 0; i < NUM_HUFF_TBLS; i++) {    cinfo->dc_huff_tbl_ptrs[i] = NULL;    cinfo->ac_huff_tbl_ptrs[i] = NULL;  }  cinfo->input_gamma = 1.0;	/* in case application forgets */  /* OK, I'm ready */  cinfo->global_state = CSTATE_START;}/* * Destruction of a JPEG compression object */GLOBAL voidjpeg_destroy_compress (j_compress_ptr cinfo){  jpeg_destroy((j_common_ptr) cinfo); /* use common routine */}/* * Forcibly suppress or un-suppress all quantization and Huffman tables. * Marks all currently defined tables as already written (if suppress) * or not written (if !suppress).  This will control whether they get emitted * by a subsequent jpeg_start_compress call. * * This routine is exported for use by applications that want to produce * abbreviated JPEG datastreams.  It logically belongs in jcparam.c, but * since it is called by jpeg_start_compress, we put it here --- otherwise * jcparam.o would be linked whether the application used it or not. */GLOBAL voidjpeg_suppress_tables (j_compress_ptr cinfo, boolean suppress){  int i;  JQUANT_TBL * qtbl;  JHUFF_TBL * htbl;  for (i = 0; i < NUM_QUANT_TBLS; i++) {    if ((qtbl = cinfo->quant_tbl_ptrs[i]) != NULL)      qtbl->sent_table = suppress;  }  for (i = 0; i < NUM_HUFF_TBLS; i++) {    if ((htbl = cinfo->dc_huff_tbl_ptrs[i]) != NULL)      htbl->sent_table = suppress;    if ((htbl = cinfo->ac_huff_tbl_ptrs[i]) != NULL)      htbl->sent_table = suppress;  }}/* * Compression initialization. * Before calling this, all parameters and a data destination must be set up. * * We require a write_all_tables parameter as a failsafe check when writing * multiple datastreams from the same compression object.  Since prior runs * will have left all the tables marked sent_table=TRUE, a subsequent run * would emit an abbreviated stream (no tables) by default.  This may be what * is wanted, but for safety's sake it should not be the default behavior: * programmers should have to make a deliberate choice to emit abbreviated * images.  Therefore the documentation and examples should encourage people * to pass write_all_tables=TRUE; then it will take active thought to do the * wrong thing. */GLOBAL voidjpeg_start_compress (j_compress_ptr cinfo, boolean write_all_tables){  if (cinfo->global_state != CSTATE_START)    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);  if (write_all_tables)    jpeg_suppress_tables(cinfo, FALSE);	/* mark all tables to be written */  /* (Re)initialize error mgr and destination modules */  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);  (*cinfo->dest->init_destination) (cinfo);  /* Perform master selection of active modules */  jinit_master_compress(cinfo);  /* Set up for the first pass */  (*cinfo->master->prepare_for_pass) (cinfo);  /* Ready for application to drive first pass through jpeg_write_scanlines */  cinfo->next_scanline = 0;  cinfo->global_state = CSTATE_SCANNING;}/* * Write some scanlines of data to the JPEG compressor. * * The return value will be the number of lines actually written. * This should be less than the supplied num_lines only in case that * the data destination module has requested suspension of the compressor, * or if more than image_height scanlines are passed in. */GLOBAL JDIMENSIONjpeg_write_scanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines,		      JDIMENSION num_lines){  JDIMENSION row_ctr, rows_left;  if (cinfo->global_state != CSTATE_SCANNING)    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);  /* Give master control module another chance if this is first call to   * jpeg_write_scanlines.  This lets output of the frame/scan headers be   * delayed so that application can write COM, etc, markers between   * jpeg_start_compress and jpeg_write_scanlines.   */  if (cinfo->master->call_pass_startup)    (*cinfo->master->pass_startup) (cinfo);  /* Ignore any extra scanlines at bottom of image. */  rows_left = cinfo->image_height - cinfo->next_scanline;  if (num_lines > rows_left)    num_lines = rows_left;  row_ctr = 0;  (*cinfo->main->process_data) (cinfo, scanlines, &row_ctr, num_lines);  cinfo->next_scanline += row_ctr;  return row_ctr;}/* * Finish JPEG compression. * * If a multipass operating mode was selected, this may do a great deal of * work including most of the actual output. */GLOBAL voidjpeg_finish_compress (j_compress_ptr cinfo){  if (cinfo->global_state != CSTATE_SCANNING ||      cinfo->next_scanline != cinfo->image_height)    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);  /* Terminate first pass */  (*cinfo->master->finish_pass) (cinfo);  /* Perform any remaining passes */  while (! cinfo->master->is_last_pass) {    (*cinfo->master->prepare_for_pass) (cinfo);    cinfo->next_scanline = 0;    while (cinfo->next_scanline < cinfo->image_height) {      (*cinfo->main->process_data) (cinfo, NULL, &cinfo->next_scanline, 0);    }    (*cinfo->master->finish_pass) (cinfo);  }  /* Write EOI, do final cleanup */  (*cinfo->marker->write_file_trailer) (cinfo);  (*cinfo->dest->term_destination) (cinfo);  /* We can use jpeg_abort to release memory and reset global_state */  jpeg_abort((j_common_ptr) cinfo);}/* * Alternate compression function: just write an abbreviated table file. * Before calling this, all parameters and a data destination must be set up. * * To produce a pair of files containing abbreviated tables and abbreviated * image data, one would proceed as follows: * *		initialize JPEG object *		set JPEG parameters *		set destination to table file *		jpeg_write_tables(cinfo); *		set destination to image file *		jpeg_start_compress(cinfo, FALSE); *		write data... *		jpeg_finish_compress(cinfo); * * jpeg_write_tables has the side effect of marking all tables written * (same as jpeg_suppress_tables(..., TRUE)).  Thus a subsequent start_compress * will not re-emit the tables unless it is passed write_all_tables=TRUE. */GLOBAL voidjpeg_write_tables (j_compress_ptr cinfo){  if (cinfo->global_state != CSTATE_START)    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);  /* (Re)initialize error mgr and destination modules */  (*cinfo->err->reset_error_mgr) ((j_common_ptr) cinfo);  (*cinfo->dest->init_destination) (cinfo);  /* Initialize the marker writer ... bit of a crock to do it here. */  jinit_marker_writer(cinfo);  /* Write them tables! */  (*cinfo->marker->write_tables_only) (cinfo);  /* And clean up. */  (*cinfo->dest->term_destination) (cinfo);  /* We can use jpeg_abort to release memory ... is this necessary? */  jpeg_abort((j_common_ptr) cinfo);}/* * Abort processing of a JPEG compression operation, * but don't destroy the object itself. */GLOBAL voidjpeg_abort_compress (j_compress_ptr cinfo){  jpeg_abort((j_common_ptr) cinfo); /* use common routine */}