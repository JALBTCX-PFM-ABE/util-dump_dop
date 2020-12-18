
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>


/* Local Includes. */

#include "FilePOSOutput.h"
#include "FileGPSOutput.h"

#include "nvutility.h"

#include "version.h"


void usage ()
{
  fprintf (stderr, "\nUsage: dump_dop [-h] PGPS_FILENAME OUTPUT_FILENAME\n");
  fprintf (stderr, "Where -h dumps HDOP instead of VDOP\n");
  fflush (stderr);
}


/********************************************************************
 *
 * Module Name : main.c
 *
 * Author/Date : Jan C. Depner
 *
 * Description : Dumps horizontal and vertical dilution of precision (DOP)
 *               data from Optech .pgps files.
 *
 ********************************************************************/

int32_t main (int32_t argc, char **argv)
{
  char               gps_file[512], pos_file[512], out_file[512];
  FILE               *gfp = NULL, *pfp = NULL, *ofp = NULL;
  GPS_OUTPUT_T       gps;
  POS_OUTPUT_T       pos;
  char               c;
  extern char        *optarg;
  extern int         optind;
  int32_t            percent = 0, prev_percent = -1, eof = 0, num_recs, year, month, day, start_week;
  int64_t            timestamp, pos_timestamp;
  float              min_hdop, max_hdop, min_vdop, max_vdop;
  uint8_t            hdop = NVFalse;
  time_t             tv_sec;
  struct tm          tm;
  static int32_t     tz_set = 0;



  int32_t option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{0, no_argument, 0, 0}};

      c = (char) getopt_long (argc, argv, "h", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              break;
            }
          break;

        case 'h':
          hdop = NVTrue;
          break;

        default:
          usage ();
          exit (-1);
          break;
        }
    }


  /* Make sure we got the mandatory file names.  */
  
  if (optind >= argc)
    {
      usage ();
      exit (-1);
    }

  strcpy (gps_file, argv[optind]);

  if (!strstr (gps_file, ".pgps"))
    {
      fprintf (stderr, "Input file %s is not a PGPS file\n", gps_file);
      exit (-1);
    }


  if ((gfp = open_gps_file (gps_file)) == NULL)
    {
      perror (gps_file);
      exit (-1);
    }


  sscanf (&gps_file[strlen (gps_file) - 16], "%02d%02d%02d", &year, &month, &day);


  /*  tm struct wants years since 1900!!!  */

  tm.tm_year = year + 100;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = 0.0;
  tm.tm_min = 0.0;
  tm.tm_sec = 0.0;
  tm.tm_isdst = -1;

  if (!tz_set)
    {
      putenv("TZ=GMT");
      tzset();
      tz_set = 1;
    }


  /*  Get seconds from the epoch (01-01-1970) for the date in the filename. 
      This will also give us the day of the week for the GPS seconds of
      week calculation.  */

  tv_sec = mktime (&tm);


  /*  Subtract the number of days since Saturday midnight (Sunday morning) in seconds.  */

  tv_sec = tv_sec - (tm.tm_wday * 86400);
  start_week = tv_sec;


  /*  Find and open the sbet (reprocessed) or pos file information.  */

  if (!get_pos_file (gps_file, pos_file))
    {
      fprintf (stderr, "Couldn't find an SBET or POS file for %s\n", gps_file);
      exit (-1);
    }

  if ((pfp = open_pos_file (pos_file)) == NULL)
    {
      fprintf (stderr, "No POS or SBET file found.\n");
      exit (-1);
    }


  /*  Open the output file.  */

  strcpy (out_file, argv[optind + 1]);
  if (strcmp (&out_file[strlen (out_file) - 4], ".trk")) strcat (out_file, ".trk");


  if ((ofp = fopen (out_file, "w+")) == NULL)
    {
      perror (out_file);
      exit (-1);
    }


  fseek (gfp, 0, SEEK_END);
  eof = ftell (gfp);
  num_recs = eof / sizeof (GPS_OUTPUT_T);
  fseek (gfp, 0, SEEK_SET);


  fprintf (stderr, "\n\n");
  fflush (stderr);


  fprintf (ofp, "MINMAX 00.0000 00.0000\n");


  min_hdop = 999.0;
  max_hdop = -999.0;
  min_vdop = 999.0;
  max_vdop = -999.0;

  while (!gps_read_record (gfp, &gps))
    {
      timestamp = (int64_t) (((double) start_week + gps.gps_time) * 1000000.0);

      if (gps.HDOP < min_hdop) min_hdop = gps.HDOP;
      if (gps.HDOP > max_hdop) max_hdop = gps.HDOP;
      if (gps.VDOP < min_vdop) min_vdop = gps.VDOP;
      if (gps.VDOP > max_vdop) max_vdop = gps.VDOP;

      pos_timestamp = pos_find_record (pfp, &pos, timestamp);

      if (pos_timestamp)
        {
          if (hdop)
            {
              fprintf (ofp, "0,0,0,%f,%f,%f\n", pos.latitude * (double) NV_RAD_TO_DEG, pos.longitude * (double) NV_RAD_TO_DEG, gps.HDOP);
            }
          else
            {
              fprintf (ofp, "0,0,0,%f,%f,%f\n", pos.latitude * (double) NV_RAD_TO_DEG, pos.longitude * (double) NV_RAD_TO_DEG, gps.VDOP);
            }
        }


      percent = ((float) ftell (gfp) / (float) eof) * 100.0;
      if (percent != prev_percent)
        {
          fprintf (stderr, "%02d%% processed \r", percent);
          fflush (stderr);
          prev_percent = percent;
        }
    }
  fprintf (stderr, "100%% processed\r");
  fflush (stderr);


  fseek (ofp, 0, SEEK_SET);
  if (hdop)
    {
      fprintf (ofp, "MINMAX %07.4f %07.4f\n", min_hdop, max_hdop);
    }
  else
    {
      fprintf (ofp, "MINMAX %07.4f %07.4f\n", min_vdop, max_vdop);
    }


  fclose (gfp);
  fclose (pfp);
  fclose (ofp);


  fprintf (stderr, "Number of records: %d\n", num_recs);
  fprintf (stderr, "Minimum HDOP: %f\n", min_hdop);
  fprintf (stderr, "Maximum HDOP: %f\n", max_hdop);
  fprintf (stderr, "Minimum VDOP: %f\n", min_vdop);
  fprintf (stderr, "Maximum VDOP: %f\n\n\n", max_vdop);
  fflush (stderr);


  return (0);
}
