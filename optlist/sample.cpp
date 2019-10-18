/***************************************************************************
*                           OptList Usage Sample
*
*   File    : sample.cpp
*   Purpose : Demonstrates usage of optlist library.
*   Author  : Michael Dipperstein
*   Date    : October 18, 2018
*
****************************************************************************
*
* Sample: A optlist library sample usage program
* Copyright (C) 2018 by
* Michael Dipperstein (mdipperstein@gmail.com)
*
* This file is part of the optlist library.
*
* The optlist library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The optlist library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <iostream>
#include "optlist.h"

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/****************************************************************************
*   Function   : main
*   Description: This is the main function for this program, it calls
*                optlist to parse the command line input displays the
*                results of the parsing.
*   Parameters : argc - number of parameters
*                argv - parameter list
*   Effects    : parses command line parameters
*   Returned   : EXIT_SUCCESS for success, otherwise EXIT_FAILURE.
****************************************************************************/
int main(int argc, char *argv[])
{
    option_t *optList, *thisOpt;

    /* get list of command line options and their arguments */
    optList = NULL;
    optList = GetOptList(argc, argv, "a:bcd:ef?");

    /* display results of parsing */
    while (optList != NULL)
    {
        thisOpt = optList;
        optList = optList->next;

        if ('?' == thisOpt->option)
        {
            std::cout << "Usage: " << FindFileName(argv[0]) << " <options>" <<
                std::endl << std::endl;
            std::cout << "options:" << std::endl;
            std::cout << "  -a : option excepting argument." << std::endl;
            std::cout << "  -b : option without arguments." << std::endl;
            std::cout << "  -c : option without arguments." << std::endl;
            std::cout << "  -d : option excepting argument." << std::endl;
            std::cout << "  -e : option without arguments." << std::endl;
            std::cout << "  -f : option without arguments." << std::endl;
            std::cout << "  -? : print out command line options.\n" <<
                std::endl;

            FreeOptList(thisOpt);   /* free the rest of the list */
            return EXIT_SUCCESS;
        }

        std::cout << "found option " << thisOpt->option << std::endl;

        if (thisOpt->argument != NULL)
        {
            std::cout << "\tfound argument " << thisOpt->argument <<
                " at index " << thisOpt->argIndex << std::endl;
        }
        else
        {
            std::cout << "\tno argument for this option" << std::endl;
        }

        free(thisOpt);    /* done with this item, free it */
    }

    return EXIT_SUCCESS;
}
