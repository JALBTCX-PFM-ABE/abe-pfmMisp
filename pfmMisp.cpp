
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmMisp.hpp"
#include "pfmMispHelp.hpp"


double settings_version = 1.0;


QListWidget      *checkList;
RUN_PROGRESS     *surfProg;


static void misp_progress_callback (char *info)
{
  if (strlen (info) >= 2)
    {
      QListWidgetItem *cur = new QListWidgetItem (QString (info));
      checkList->addItem (cur);
      checkList->setCurrentItem (cur);
      checkList->scrollToItem (cur);
    }

  qApp->processEvents ();
}



pfmMisp::pfmMisp (int32_t *argc, char **argv, QWidget *parent)
  : QWizard (parent, 0)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/pfmMispWatermark.png"));


  //  Get the user's defaults if available

  envin (&options);


  // Set the application font

  QApplication::setFont (options.font);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, true);
  setOption (ExtendedWatermarkPixmap, false);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (options.window_width, options.window_height);
  this->move (options.window_x, options.window_y);


  setPage (0, new startPage (argc, argv, this, &options));

  setPage (1, new surfacePage (this, &options));

  setPage (2, new runPage (this, &progress, &checkList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, true);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start generating the surface"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


pfmMisp::~pfmMisp ()
{
}



void pfmMisp::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.png"));
  button (QWizard::CustomButton1)->setEnabled (false);


  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      button (QWizard::CustomButton1)->setEnabled (true);

      pfm_file_name = field ("pfm_file_edit").toString ();

      options.clear_int = field ("nFlag").toBool ();
      options.nibble = field ("nibble").toInt ();
      options.clear_land = field ("clearLand").toBool ();
      options.replace_all = field ("replaceAll").toBool ();
      options.weight = field ("factor").toInt ();
      options.force_original_value = field ("force").toBool ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      options.window_x = tmp.x ();
      options.window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      options.window_width = tmp.width ();
      options.window_height = tmp.height ();


      //  Save the options.

      envout (&options);


      QString string;

      checkList->clear ();

      string = tr ("PFM file : ") + pfm_file_name;
      checkList->addItem (string);

      switch (options.surface)
        {
        case 0:
          string = tr ("Grid Minimum Filtered Surface");
          checkList->addItem (string);
          break;

        case 1:
          string = tr ("Grid Maximum Filtered Surface");
          checkList->addItem (string);
          break;

        case 2:
          string = tr ("Grid all depth values");
          checkList->addItem (string);
          break;
        }


      switch (options.replace_all)
        {
        case NVFalse:
          string = tr ("Replace only empty bins with interpolated value");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Replace all bins with interpolated value");
          checkList->addItem (string);
          break;
        }

      switch (options.clear_land)
        {
        case NVFalse:
          string = tr ("Don't clear SRTM masked land");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Clear SRTM masked land");
          checkList->addItem (string);
          break;
        }


      switch (options.force_original_value)
        {
        case NVFalse:
          string = tr ("Don't force bins to original input value");
          checkList->addItem (string);
          break;

        case NVTrue:
          string = tr ("Force bins to original input value");
          checkList->addItem (string);
          break;
        }

      string = QString (tr ("MISP weight factor : %1")).arg (options.weight);
      checkList->addItem (string);


      if (options.clear_int)
        {
          string = QString (tr ("Nibbler value (bins) : %1")).arg (options.nibble);
          checkList->addItem (string);
        }

      break;
    }
}



void pfmMisp::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



void pfmMisp::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  This is where the fun stuff happens.

void 
pfmMisp::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  int32_t             recnum, pfm_handle, out_count = 0, dn, up, bw, fw;
  float               *array;
  double              lat, lon;
  NV_F64_XYMBR        mbr;
  NV_F64_COORD3       xyz;
  BIN_RECORD          bin;
  DEPTH_RECORD        *depth;
  NV_I32_COORD2       coord;
  PFM_OPEN_ARGS       open_args;
  uint8_t             found = NVFalse, land_mask_flag = NVFalse;
  uint32_t            **val_array = NULL;


  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (false);
  button (QWizard::BackButton)->setEnabled (false);
  button (QWizard::CustomButton1)->setEnabled (false);


  strcpy (open_args.list_path, pfm_file_name.toLatin1 ());

  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);

  if (pfm_handle < 0) pfm_error_exit (pfm_error);


  //  Check for the land mask flag in PFM_USER_10 in any of the PFM layers.

  land_mask_flag = NVFalse;
  if (!strcmp (open_args.head.user_flag_name[9], "Land masked point")) land_mask_flag = NVTrue;


  /*  We're going to let MISP handle everything in zero based units of the bin size.  That is, we subtract off the
      west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
      the south latitude.  This will give us values that range from 0.0 to gridcols in longitude and 0.0 to gridrows
      in latitude.  The assumption here is that the bins are essentially squares (spatially).  */

  mbr.min_x = 0.0;
  mbr.min_y = 0.0;
  if (open_args.head.proj_data.projection)
    {
      mbr.max_x = (double) NINT ((open_args.head.mbr.max_x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy);
      mbr.max_y = (double) NINT ((open_args.head.mbr.max_y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy);
    }
  else
    {
      mbr.max_x = (double) NINT ((open_args.head.mbr.max_x - open_args.head.mbr.min_x) /
                                     open_args.head.x_bin_size_degrees);
      mbr.max_y = (double) NINT ((open_args.head.mbr.max_y - open_args.head.mbr.min_y) /
                                     open_args.head.y_bin_size_degrees);
    }


  //  Register the progress callback with MISP.

  misp_register_progress_callback (misp_progress_callback);

  misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, options.weight, mbr);


  progress.gbox->setTitle (tr ("Reading data for surface"));

  progress.gbar->setRange (0, open_args.head.bin_height);


  for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
    {
      coord.y = i;

      for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
        {
          coord.x = j;

          read_bin_record_index (pfm_handle, coord, &bin);

          if (bin.num_soundings)
            {
              if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                {
                  found = NVFalse;
                  for (int32_t k = 0 ; k < recnum ; k++)
                    {
                      switch (options.surface)
                        {
                        case 2:
                          if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                            {
                              out_count++;
    
                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }

                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);
                            }
                          break;
    
    
                        case 0:
                          if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) && 
                              fabs (depth[k].xyz.z - bin.min_filtered_depth) < MISP_EPS)
                            {
                              out_count++;

                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }
    
                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);

                              found = NVTrue;
                            }
                          break;
    
    
                        case 1:
                          if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) &&
                              fabs (depth[k].xyz.z - bin.max_filtered_depth) < MISP_EPS)
                            {
                              out_count++;
    
                              if (open_args.head.proj_data.projection)
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.bin_size_xy;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.bin_size_xy;
                                }
                              else
                                {
                                  xyz.x = (depth[k].xyz.x - open_args.head.mbr.min_x) / open_args.head.x_bin_size_degrees;
                                  xyz.y = (depth[k].xyz.y - open_args.head.mbr.min_y) / open_args.head.y_bin_size_degrees;
                                }
    
                              xyz.z = depth[k].xyz.z;

                              misp_load (xyz);

                              found = NVTrue;
                            }
                          break;
                        }
                      if (found) break;
                    }
                  free (depth);
                }
            }
        }


      progress.gbar->setValue (i);

      qApp->processEvents ();
    }
  progress.gbar->reset ();


  progress.gbox->setTitle (tr ("Generating grid surface"));


  //  Setting range to 0, 0 makes the bar just show movement.

  progress.gbar->setRange (0, 0);

  qApp->processEvents ();


  if (misp_proc ()) exit (-1);


  if (options.replace_all)
    {
      switch (options.surface)
        {
        case 2:
          strcpy (open_args.head.average_filt_name, "AVERAGE MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;

        case 0:
          strcpy (open_args.head.average_filt_name, "MINIMUM MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;

        case 1:
          strcpy (open_args.head.average_filt_name, "MAXIMUM MISP SURFACE");
          write_bin_header (pfm_handle, &open_args.head, 0);
          break;
        }
    }


  progress.gbar->setRange (0, open_args.head.bin_height);

  progress.gbox->setTitle (tr ("Retrieving gridded surface data"));


  /*  Allocating one more column than we need due to chrtr specific changes in misp_rtrv (see misp_funcs.c).  */

  array = (float *) malloc ((open_args.head.bin_width + 1) * sizeof (float));

  if (array == NULL)
    {
      perror ("Allocating array");
      exit (-1);
    }


  for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
    {
      //  Compute the latitude of the bin.

      NV_F64_COORD2 xy;
      xy.y = open_args.head.mbr.min_y + i * open_args.head.y_bin_size_degrees;


      coord.y = i;

      if (!misp_rtrv (array)) break;

      for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
        {
          //  Compute the longitude of the bin.

          xy.x = open_args.head.mbr.min_x + j * open_args.head.x_bin_size_degrees;


          //  Don't try to write points that fall outside of the PFM polygon (it might not be a rectangle).

          if (bin_inside_ptr (&open_args.head, xy))
            {
              coord.x = j;


              read_bin_record_index (pfm_handle, coord, &bin);


              //  This is a special case.  We don't want to replace land masked bins unless the land mask point has been
              //  deleted.

              if (!land_mask_flag || !(bin.validity & PFM_DATA) || !(bin.validity & PFM_USER_10))
                {
                  if (options.replace_all || !(bin.validity & PFM_DATA)) 
                    {
                      bin.validity |= PFM_INTERPOLATED;
                      if (array[j] <= open_args.max_depth && array[j] > -open_args.offset)
                        {
                          bin.avg_filtered_depth = array[j];
                        }
                      else
                        {
                          bin.avg_filtered_depth = open_args.head.null_depth;
                        }


                      //  If there was a land mask point in this bin and it has been deleted, unset the 
                      //  PFM_USER_10 flag.

                      if (bin.validity & PFM_USER_10) bin.validity &= ~PFM_USER_10;


                      //  If we set the nibble argument to 0 we don't want to put interpolated values into
                      //  empty bins.

                      if ((bin.validity & PFM_DATA) || !options.clear_int || (options.clear_int && options.nibble)) write_bin_record_index (pfm_handle, &bin);
                    }
                }
            }
        }

      progress.gbar->setValue (i);

      qApp->processEvents ();
    }

  free (array);

  progress.gbar->setValue (open_args.head.bin_height);


  qApp->processEvents ();


  /*  Nibble out the cells that aren't within our optional nibbling distance from a cell with valid data.  */

  if (options.clear_int && options.nibble)
    {
      /*  This can get into a fair amount of memory but it is the most
          efficient way to do this since we're doing, basically, sequential
          I/O on both ends.  If you have a 2000 by 2000 cell bin this will
          allocate 16MB of memory.  Hopefully your machine can handle that.
          If not, buy a new machine.  */

      val_array = (uint32_t **) calloc (open_args.head.bin_height, sizeof (uint32_t *));

      if (val_array == NULL)
        {
          perror ("Allocating val_array");
          exit (-1);
        }



      for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
        {
          val_array[i] = (uint32_t *) calloc (open_args.head.bin_width, sizeof (uint32_t));

          if (val_array[i] == NULL)
            {
              perror ("Allocating memory for nibbler");
              exit (-1);
            }
        }


      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing interpolated data (reading validity)"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              read_bin_record_validity_index (pfm_handle, coord, &val_array[i][j]);
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();


      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing interpolated data (nibbling)"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          dn = MAX (i - options.nibble, 0);
          up = MIN (i + options.nibble, open_args.head.bin_height - 1);

          for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              if (!(val_array[i][j] & PFM_DATA))
                {
                  bw = MAX (j - options.nibble, 0);
                  fw = MIN (j + options.nibble, open_args.head.bin_width - 1);

                  found = NVFalse;
                  for (int32_t k = dn ; k <= up ; k++)
                    {
                      for (int32_t m = bw ; m <= fw ; m++)
                        {
                          if (val_array[k][m] & PFM_DATA)
                            {
                              found = NVTrue;
                              break;
                            }
                        }
                      if (found) break;
                    }

                  if (!found)
                    {
                      bin.coord = coord;
                      bin.validity = val_array[i][j] & (~PFM_INTERPOLATED);
                      write_bin_record_validity_index (pfm_handle, &bin, PFM_INTERPOLATED);
                    }
                }
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();
    }


  /*  Clear landmasked data if requested.  */

  if (options.clear_land)
    {
      progress.gbar->reset ();

      progress.gbox->setTitle (tr ("Clearing SRTM land data"));

      progress.gbar->setRange (0, open_args.head.bin_height);


      for (int32_t i = 0 ; i < open_args.head.bin_height ; i++)
        {
          coord.y = i;

          lat = open_args.head.mbr.min_y + ((double) i + 0.5) * open_args.head.y_bin_size_degrees;

          for (int32_t j = 0 ; j < open_args.head.bin_width ; j++)
            {
              coord.x = j;

              lon = open_args.head.mbr.min_x + ((double) j + 0.5) * open_args.head.x_bin_size_degrees;

              read_bin_record_index (pfm_handle, coord, &bin);


              /*  If there's real data in the cell don't believe the mask.  */

              if (!(bin.num_soundings))
                {
                  if (read_srtm_mask (lat, lon) == 1)
                    {
                      bin.validity &= (~PFM_INTERPOLATED);
                      write_bin_record_index (pfm_handle, &bin);
                    }
                }
            }

          progress.gbar->setValue (i);

          qApp->processEvents ();
        }

      progress.gbar->setValue (open_args.head.bin_height);

      qApp->processEvents ();
    }

  close_pfm_file (pfm_handle);


  button (QWizard::FinishButton)->setEnabled (true);
  button (QWizard::CancelButton)->setEnabled (false);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  checkList->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Gridding complete, press Finish to exit."));

  checkList->addItem (cur);
  checkList->setCurrentItem (cur);
  checkList->scrollToItem (cur);
}



//  Get the users defaults.

void pfmMisp::envin (OPTIONS *options)
{
  //  We need to get the font from the global settings.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  options->font = QApplication::font ();

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = options->font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  options->font.fromString (fontString);


  settings2.endGroup ();


  double saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  options->clear_land = NVFalse;
  options->replace_all = NVFalse;
  options->force_original_value = NVFalse;
  options->surface = 2;
  options->clear_int = 0;
  options->nibble = 0;
  options->weight = 2;
  options->input_dir = ".";
  options->window_x = 0;
  options->window_y = 0;
  options->window_width = 800;
  options->window_height = 400;


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/pfmMisp.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/pfmMisp.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("pfmMisp");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->clear_land = settings.value (QString ("clear land"), options->clear_land).toBool ();

  options->replace_all = settings.value (QString ("replace all"), options->replace_all).toBool ();

  options->force_original_value = settings.value (QString ("force original value"), options->force_original_value).toBool ();

  options->surface = settings.value (QString ("surface"), options->surface).toInt ();

  options->clear_int = settings.value (QString ("clear interpolated"), options->clear_int).toBool ();
  options->nibble = settings.value (QString ("nibble value"), options->nibble).toInt ();

  options->weight = settings.value (QString ("weight"), options->weight).toInt ();

  options->input_dir = settings.value (QString ("input directory"), options->input_dir).toString ();

  options->window_width = settings.value (QString ("width"), options->window_width).toInt ();
  options->window_height = settings.value (QString ("height"), options->window_height).toInt ();
  options->window_x = settings.value (QString ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (QString ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void pfmMisp::envout (OPTIONS *options)
{
  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/pfmMisp.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/pfmMisp.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("pfmMisp");

  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("clear land"), options->clear_land);

  settings.setValue (QString ("replace all"), options->replace_all);

  settings.setValue (QString ("force original value"), options->force_original_value);

  settings.setValue (QString ("surface"), options->surface);

  settings.setValue (QString ("clear interpolated"), options->clear_int);
  settings.setValue (QString ("nibble value"), options->nibble);

  settings.setValue (QString ("weight"), options->weight);

  settings.setValue (QString ("input directory"), options->input_dir);

  settings.setValue (QString ("width"), options->window_width);
  settings.setValue (QString ("height"), options->window_height);
  settings.setValue (QString ("x position"), options->window_x);
  settings.setValue (QString ("y position"), options->window_y);

  settings.endGroup ();
}
