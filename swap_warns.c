
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

/*****************************************************************************
 * swap_warns CSU
 *
 * Purpose:  	This is a routine will swap all the warnings that are 
 *		associated with a "first" or "second" depth".
 *	
 * ARGs:	warnings	-the first warning word.
 * 		warnings2	-the second warning word.
 *
 *	FYI:
 *		AU2_DLESSONE_FLAG_MASK		2	
 *		AU2_D2LESSONE_FLAG_MASK		4
 *
 * Modifications:
 * 97/11/05 DR	-wrote routine
 * 
 * 97/11/07 DR	-GG9709
 * 		-added code so when a "LESSER_PULSE_SELECTED" flag is set
 *		 for a pulse that is to be swapped, it will be removed
 *		 after the swap.  If it is swapped back it will not
 *		 come back...
 *****************************************************************************/
#include <stdio.h>
#include "warns.h"

void	swap_first_second_warns(int,int*,int,int*);

void	swap_warns(int *warnings, int *warnings2)
{
	int	dummy;
	int	*dummy_ptr;
	dummy = 0;
	dummy_ptr = &dummy;

	swap_first_second_warns(AU2_DLESSONE_FLAG_MASK, &(*warnings2), 
		AU2_D2LESSONE_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_RAMAN_BOTTOM_FLAG_MASK, &(*warnings), 
		AU2_SEC_RAMAN_BOTTOM_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_PEAK_FIFTY_ERROR_FLAG_MASK, &(*warnings), 
		AU2_SEC_PEAK_FIFTY_ERROR_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_SATURATED_APD_BOT_FLAG_MASK, &(*warnings), 
		AU2_SEC_SAT_APD_BOT_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_STRETCHED_BOT_FLAG_MASK, &(*warnings), 
		AU2_SEC_STRETCHED_BOT_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_SHOAL_PEAK_FLAG_MASK, &(*warnings), 
		AU2_SEC_SHOAL_PEAK_FLAG_MASK, &(*warnings2));

	swap_first_second_warns(AU_LESSER_PULSE_SELECTED_FLAG_MASK,&(*warnings),
		0, &(*dummy_ptr));
}

/*****************************************************************************
 * swap_first_second_warns CSU
 *
 * Purpose:  	This is a generic routine that will swap a pair of 
 *		warnings (if they are set).  In a case where both
 *		warnings are in the same warning word, that word
 *		can be passed as both the second and fourth arguement.
 *	
 * ARGs:	flag_mask1	-the "first" warning to be swapped
 * 		warnings	-the warning word wher the "first" warning
 *		 		 is found.
 * 		flag_mask1	-the "first" warning to be swapped
 * 		warnings2	-the warning word wher the "second" warning
 *		 		 is found.
 *
 *	FYI:
 *		AU2_DLESSONE_FLAG_MASK		2	
 *		AU2_D2LESSONE_FLAG_MASK		4
 *
 * Modifications:
 * 97/11/05 DR	-wrote routine
 * 
 *****************************************************************************/


void
swap_first_second_warns( int flag_mask1, int* warnings, int flag_mask2, int *warnings2 )
{
	int	first, second;

	first = 0;
	second = 0;

	/* Check for DLESSONE / D2LESSONE */
	if(*warnings & flag_mask1)
	{
		first = 1;
		*warnings ^= flag_mask1;
	}

	if(*warnings2 & flag_mask2)
	{
		second = 1;
		*warnings2 ^= flag_mask2;
	}

	if(first)
		*warnings2 |= flag_mask2;

	if(second)
		*warnings |= flag_mask1;

}

