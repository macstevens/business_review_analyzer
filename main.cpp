#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include "business_review_analyzer.h"



/*
command line:
  business_review_analyzer.exe --start-date-time 2012-01-01T12:00:00 --end-date-time 2024-04-20T12:00:00 file1.txt file2.htm file3.txt 
*/
int main( int argc, char *argv[] )
{
int result = business_review_analyzer::run_main(argc, argv);
return result;
}
