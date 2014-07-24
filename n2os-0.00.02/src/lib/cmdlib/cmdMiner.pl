#! /usr/bin/perl
##
##
##
no warnings;
####################################################### [ CHECK DUPLICATE ] ##########################################################
my @nosDupTable;
#DECMD|ALICMD
foreach (@ARGV)
{
  $file = $_;
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments

  @defun = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defun)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fcmdstr = "$defun_array[5]";
    $fcmdstr=~ s/^\s+//g;
    $fcmdstr =~ s/\s+$//g;
    # Check Existed
    foreach (@nosDupTable)
    {
      if ($_ eq $fcmdstr)
      {
        print "Duplicate string Command line $_\n";
        exit 1;
      }
    }
    #
    push (@nosDupTable, $fcmdstr);
  }

  @defun = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);

  foreach (@defun)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fcmdstr = "$defun_array[3]";
    $fcmdstr=~ s/^\s+//g;
    $fcmdstr =~ s/\s+$//g;
    # Check Existed 
    foreach (@nosDupTable) 
    {
      if ($_ eq $fcmdstr)
      {
        print "Duplicate string Command line \"$_\" in file \"$file\"";
        exit 1;
      }
    }
  }
}
####################################################### [nnGlobalCmd.h] ##############################################################
open (WGLOBALFILE_H, '>nnGlobalCmd.h');
printf WGLOBALFILE_H "#ifndef __NN_GLOBAL_CMD_H__\n";
printf WGLOBALFILE_H "#define __NN_GLOBAL_CMD_H__\n";
printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_WRITE_FUNC_ID", $index;
$index = 0;
foreach (@ARGV)
{
  $file = $_;
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  printf WGLOBALFILE_H "\n\n/**\t\t[%s]\t\t**/\n\n", $file;
  $file =~ s{.*/}{};      # removes path
  $file =~ s{\.[^.]+$}{}; # removes extension
  $ucFile = uc $file;
  $gIndexBase = $index*1000 + 1;
  #RM COMMENTS
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
  $gIndexFunc = 0;
  #DENODEC
  @defunc = ($line =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DENODEC]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $fname = uc $fname;
    printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID", $gIndexBase + $gIndexFunc;
    $gIndexFunc++;
  }
  #DECMD
  @defunc = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DECMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $fname = uc $fname;
    printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID", $gIndexBase + $gIndexFunc;
    $gIndexFunc++;
  }
  #ALICMD
  @defunc = ($line =~ /(?:ALICMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[ALICMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $fname = uc $fname;
    printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID", $gIndexBase + $gIndexFunc;
    $gIndexFunc++;
  }
  #DENODE
  @defunc = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DENODE]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $fname = uc $fname;
    printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID", $gIndexBase + $gIndexFunc; #Go To Node
    $gIndexFunc++;
    printf WGLOBALFILE_H "%-50s %4d\n", "#define CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID", $gIndexBase + $gIndexFunc; #Exit  Node
    $gIndexFunc++;
  }
  $index++;
}

printf WGLOBALFILE_H "#endif\t/** __NN_GLOBAL_CMD_H__ */\n";
close(WGLOBALFILE_H);

####################################################### [nnGlobalCmd.c] ##############################################################
open (WGLOBALFILE_C, '>nnGlobalCmd.c');
printf WGLOBALFILE_C "/*\n";
printf WGLOBALFILE_C " * \@file      :  cmdGlobalCli.c  \n";
printf WGLOBALFILE_C " * \@brief       :  \\n";
printf WGLOBALFILE_C " * \\n";
printf WGLOBALFILE_C " * \$Id: cmdGlobalCli.c 863 2014-02-18 06:04:16Z thanh $ \\n";
printf WGLOBALFILE_C " * \$Author: thanh $ \\n";
printf WGLOBALFILE_C " * \$Date: 2014-02-18 01:04:16 -0500 (Tue, 18 Feb 2014) $ \\n";
printf WGLOBALFILE_C " * \$Log$ \\n";
printf WGLOBALFILE_C " * \$Revision: 863 $ \\n";
printf WGLOBALFILE_C " * \$LastChangedBy: thanh $ \\n";
printf WGLOBALFILE_C " * \$LastChanged$ \n";
printf WGLOBALFILE_C " * \n";
printf WGLOBALFILE_C " *                      Electronics and Telecommunications Research Institute\n";
printf WGLOBALFILE_C " * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.\n";
printf WGLOBALFILE_C " *           No part of this software shall be reproduced, stored in a retrieval system, or\n";
printf WGLOBALFILE_C " *           transmitted by any means, electronic, mechanical, photocopying, recording,\n";
printf WGLOBALFILE_C " *           or otherwise, without written permission from ETRI.\n";
printf WGLOBALFILE_C " */\n";
printf WGLOBALFILE_C "#include <stdio.h>\n";
printf WGLOBALFILE_C "#include <stdarg.h>\n";
printf WGLOBALFILE_C "#include <stdlib.h>\n";
printf WGLOBALFILE_C "#include <string.h>\n";
printf WGLOBALFILE_C "#include \"nnTypes.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdCommon.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdNode.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdCmsh.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdCli.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdDefines.h\"\n";
printf WGLOBALFILE_C "#include \"nnCmdInstall.h\"\n";
printf WGLOBALFILE_C "#include \"nnGlobalCmd.h\"\n";

printf WGLOBALFILE_C "void\n";
printf WGLOBALFILE_C "cmdInitElements(struct cmdElement *cel, Int8T *cmdStr, ...)\n";
printf WGLOBALFILE_C "{\n";
printf WGLOBALFILE_C "  va_list vl;\n";
printf WGLOBALFILE_C "  Int32T i;\n";
printf WGLOBALFILE_C "  va_start(vl,cmdStr);\n";
printf WGLOBALFILE_C "  cel->helpNum = cmdGetTokenNumHelp(cmdStr);\n";
printf WGLOBALFILE_C "  cel->help = malloc(cel->helpNum * sizeof(char*));\n";
printf WGLOBALFILE_C "  for( i = 0; i < cel->helpNum; i++)\n";
printf WGLOBALFILE_C "  {\n";
printf WGLOBALFILE_C "    cel->help[i] = va_arg(vl,Int8T *);\n";
printf WGLOBALFILE_C "  }\n";
printf WGLOBALFILE_C "  va_end(vl);\n";
printf WGLOBALFILE_C "}\n";

$index = 0;
foreach (@ARGV)
{
  $file = $_;
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  printf WGLOBALFILE_C "\n\n/**\t\t[%s]\t\t**/\n\n", $file;
  $file =~ s{.*/}{};      # removes path
  $file =~ s{\.[^.]+$}{}; # removes extension
  $ucFile = uc $file;
  $gIndexBase = $index*1000 + 1;
  #RM COMMENTS
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
  $gIndexFunc = 0;
  #DENODEC
  @defunc = ($line =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defunc)
  {
    $gIndexFunc++;
  }
  #DECMD
  @defunc = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_C "/**\t[DECMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[3]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $ucName = uc $fname;
    printf WGLOBALFILE_C "struct cmdElement nosCmd_%d_CEL = \n", $gIndexBase+$gIndexFunc;
    printf WGLOBALFILE_C "{\n";
    printf WGLOBALFILE_C "\t$fname,\n";
    printf WGLOBALFILE_C "\t%s\n", "CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID";
    printf WGLOBALFILE_C "};\n";
    $gIndexFunc++;
  }
  #ALICMD
  @defunc = ($line =~ /(?:ALICMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_C "/**\t[ALICMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[3]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $ucName = uc $fname;
    printf WGLOBALFILE_C "struct cmdElement nosCmd_%d_CEL = \n", $gIndexBase+$gIndexFunc;
    printf WGLOBALFILE_C "{\n";
    printf WGLOBALFILE_C "\t$fname,\n";
    printf WGLOBALFILE_C "\t%s\n", "CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID";
    printf WGLOBALFILE_C "};\n";
    $gIndexFunc++;
  }
  #DENODE
  @defunc = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_C "/**\t[DENODE]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $fname = "$defunc_array[5]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $ucName = uc $fname;
    printf WGLOBALFILE_C "struct cmdElement nosCmd_%d_CEL = \n", $gIndexBase+$gIndexFunc;
    printf WGLOBALFILE_C "{\n";
    printf WGLOBALFILE_C "\t$fname,\n";
    printf WGLOBALFILE_C "\t%s\n", "CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID";
    printf WGLOBALFILE_C "};\n";
    $gIndexFunc++;
    printf WGLOBALFILE_C "struct cmdElement nosCmd_%d_CEL = \n", $gIndexBase+$gIndexFunc;
    printf WGLOBALFILE_C "{\n";
    printf WGLOBALFILE_C "\t\"exit\",\n";
    printf WGLOBALFILE_C "\t%s\n", "CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID";
    printf WGLOBALFILE_C "};\n";
    $gIndexFunc++;
  }
  $index++;
}

printf WGLOBALFILE_C "void\n";
printf WGLOBALFILE_C "cmdGlobalTreeInstall(struct cmsh *cmsh)\n";
printf WGLOBALFILE_C "{\n";
printf WGLOBALFILE_C "\tInt32T cmdGlobalFlags[MAX_PROCESS_CNT];\n";

$index = 0;
foreach (@ARGV)
{
  $file = $_;
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  printf WGLOBALFILE_C "\n\n/**\t\t[%s]\t\t**/\n\n", $file;
  $file =~ s{.*/}{};      # removes path
  $file =~ s{\.[^.]+$}{}; # removes extension
  $ucFile = uc $file;
  $gIndexBase = $index*1000 + 1;
  #RM COMMENTS
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
  $gIndexFunc = 0;
  #DENODEC
  @defunc = ($line =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defunc)
  {
    $gIndexFunc++;
  }  
  #DECMD
  @defunc = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DECMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $num = 0;
    foreach(@defunc_array)
    {
      $num++;
    }
    $fcmdstr = "$defunc_array[3]";
    $fcmdstr=~ s/^\s+//g;
    $fcmdstr =~ s/\s+$//g;
    printf WGLOBALFILE_C "\tcmdInitElements(&nosCmd_%d_CEL, %s", $gIndexBase+$gIndexFunc, $fcmdstr;
    for($count = 4; $count < $num; $count++)
    {
      $fhelp = "$defunc_array[$count]";
      $fhelp =~ s/^\s+//g;
      $fhelp =~ s/\s+$//g;
      printf WGLOBALFILE_C ",\n\t\t%s", $fhelp;
    }
    printf WGLOBALFILE_C ");\n";
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;

    $fnode = "$defunc_array[1]";
    $fnode =~ s/^\s+//g;
    $fnode =~ s/\s+$//g;

    $fflags = "$defunc_array[2]";
    $fflags =~ s/^\s+//g;
    $fflags =~ s/\s+$//g;
    $fflags =~ s/ //g;
    my @charsFlags = split('\|', $fflags);
    $flagsIndex = 0;
    foreach my $val (@charsFlags) {
      print WGLOBALFILE_C "\tcmdGlobalFlags[$flagsIndex] = $val;\n";
      $flagsIndex++;
    }
    printf WGLOBALFILE_C "\tcmdCommandInstall(cmsh, $fnode, cmdGlobalFlags,  $flagsIndex, &nosCmd_%d_CEL);\n", $gIndexBase+$gIndexFunc;
    $gIndexFunc++;
  }
  #ALICMD
  @defunc = ($line =~ /(?:ALICMD)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[ALICMD]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defunc_array);
    @defunc_array = split (/,/);
    $num = 0;
    foreach(@defunc_array)
    {
      $num++;
    }
    $fcmdstr = "$defunc_array[3]";
    $fcmdstr=~ s/^\s+//g;
    $fcmdstr =~ s/\s+$//g;
    printf WGLOBALFILE_C "\tcmdInitElements(&nosCmd_%d_CEL, %s", $gIndexBase+$gIndexFunc, $fcmdstr;
    for($count = 4; $count < $num; $count++)
    {
      $fhelp = "$defunc_array[$count]";
      $fhelp =~ s/^\s+//g;
      $fhelp =~ s/\s+$//g;
      printf WGLOBALFILE_C ",\n\t\t%s", $fhelp;
    }
    printf WGLOBALFILE_C ");\n";
    $fname = "$defunc_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;

    $fnode = "$defunc_array[1]";
    $fnode =~ s/^\s+//g;
    $fnode =~ s/\s+$//g;

    $fflags = "$defunc_array[2]";
    $fflags =~ s/^\s+//g;
    $fflags =~ s/\s+$//g;
    $fflags =~ s/ //g;
    my @charsFlags = split('\|', $fflags);
    $flagsIndex = 0;
    foreach my $val (@charsFlags) {
      print WGLOBALFILE_C "\tcmdGlobalFlags[$flagsIndex] = $val;\n";
      $flagsIndex++;
    }
    printf WGLOBALFILE_C "\tcmdCommandInstall(cmsh, $fnode, cmdGlobalFlags,  $flagsIndex, &nosCmd_%d_CEL);\n", $gIndexBase+$gIndexFunc;
    $gIndexFunc++;
  }
  #DENODE
  @defunc = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DENODE]\t*/\n";
  }
  foreach (@defunc)
  {
    $gIndexFunc++;
  }
  $index++;
}
printf WGLOBALFILE_C "}\n";

print WGLOBALFILE_C "void\n";
print WGLOBALFILE_C "cmdGlobalNodeInstall(struct cmsh *cmsh)\n";
print WGLOBALFILE_C "{\n";
$index = 0;
foreach (@ARGV)
{
  $file = $_;
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  printf WGLOBALFILE_C "\n\n/**\t\t[%s]\t\t**/\n\n", $file;
  $file =~ s{.*/}{};      # removes path
  $file =~ s{\.[^.]+$}{}; # removes extension
  $ucFile = uc $file;
  $gIndexBase = $index*1000 + 1;
  #RM COMMENTS
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
  $gIndexFunc = 0;
  #DENODEC
  @defunc = ($line =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defunc)
  {
    $gIndexFunc++;
  }
  #DECMD
  @defunc = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defunc)
  {
    $gIndexFunc++;
  }
  #ALICMD
  @defunc = ($line =~ /(?:ALICMD)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@defunc)
  {
    $gIndexFunc++;
  }
  #DENODE
  @defunc = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@defunc)
  {
    printf WGLOBALFILE_H "/**\t[DENODE]\t*/\n";
  }
  foreach (@defunc)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $num = 0;
    foreach(@defun_array)
    {
      $num++;
    }
    $fcmdstr = "$defun_array[5]";
    $fcmdstr=~ s/^\s+//g;
    $fcmdstr =~ s/\s+$//g;

    printf WGLOBALFILE_C "\tcmdInitElements(&nosCmd_%d_CEL, %s", $gIndexBase+$gIndexFunc, $fcmdstr;
    for($count = 6; $count < $num; $count++)
    {
      $fhelp = "$defun_array[$count]";
      $fhelp =~ s/^\s+//g;
      $fhelp =~ s/\s+$//g;
      printf WGLOBALFILE_C ", %s", $fhelp;
    }
    printf WGLOBALFILE_C ");\n";
    $fname = "$defun_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;

    $fnode = "$defun_array[1]";
    $fnode =~ s/^\s+//g;
    $fnode =~ s/\s+$//g;

    $fpnode = "$defun_array[2]";
    $fpnode =~ s/^\s+//g;
    $fpnode =~ s/\s+$//g;

    $fstr = "$defun_array[3]";
    $fstr =~ s/^\s+//g;
    $fstr =~ s/\s+$//g;

    $fpriv = "$defun_array[4]";
    $fpriv =~ s/^\s+//g;
    $fpriv =~ s/\s+$//g;
    $gpNodeIndex = $gIndexBase+$gIndexFunc;
    $gIndexFunc++;
    $geNodeIndex = $gIndexBase+$gIndexFunc;
    printf WGLOBALFILE_C "\tcmdInitElements(&nosCmd_%d_CEL, \"exit\", \"Exit Node\");", $geNodeIndex, $fcmdstr;
    printf WGLOBALFILE_C "\n\tcmdNodeInstall(cmsh, $fpnode, $fnode, $fpriv, $fstr, &nosCmd_%d_CEL, &nosCmd_%d_CEL);\n", $gpNodeIndex, $geNodeIndex;
    $gIndexFunc++;
  }
  $index++;
}
printf WGLOBALFILE_C "}\n";

print WGLOBALFILE_C "void\n";
print WGLOBALFILE_C "cmdGlobalInstall(struct cmsh *cmsh)\n";
print WGLOBALFILE_C "{\n";
print WGLOBALFILE_C "\tcmdGlobalNodeInstall(cmsh);\n";
print WGLOBALFILE_C "\tcmdGlobalTreeInstall(cmsh);\n";
print WGLOBALFILE_C "}\n";

close(WGLOBALFILE_C);

####################################################### [xxxCmd_Gen.c] ##############################################################
$index = 0;
foreach (@ARGV)
{
  $file = $_;
  $filew = $_;
  $filew =~ s/\.c/_GEN\.c/g;
  open (WGLOBALFILE_GEN_C, "> $filew");
  open (PTR, "< $file");
  local $/; undef $/;
  $line = <PTR>;
  close (PTR);
  $file =~ s{.*/}{};      # removes path
  $file =~ s{\.[^.]+$}{}; # removes extension
  $ucFile = uc $file;
  $gIndexBase = $index*1000 + 1;
  #WRITE HEADER
  printf WGLOBALFILE_GEN_C "/*\n";
  printf WGLOBALFILE_GEN_C " * \@file      :  $filew  \n";
  printf WGLOBALFILE_GEN_C " * \@brief       :  \\n";
  printf WGLOBALFILE_GEN_C " * \\n";
  printf WGLOBALFILE_GEN_C " * \$Id: cmdGlobalCli.c 863 2014-02-18 06:04:16Z thanh $ \\n";
  printf WGLOBALFILE_GEN_C " * \$Author: thanh $ \\n";
  printf WGLOBALFILE_GEN_C " * \$Date: 2014-02-18 01:04:16 -0500 (Tue, 18 Feb 2014) $ \\n";
  printf WGLOBALFILE_GEN_C " * \$Log$ \\n";
  printf WGLOBALFILE_GEN_C " * \$Revision: 863 $ \\n";
  printf WGLOBALFILE_GEN_C " * \$LastChangedBy: thanh $ \\n";
  printf WGLOBALFILE_GEN_C " * \$LastChanged$ \n";
  printf WGLOBALFILE_GEN_C " * \n";
  printf WGLOBALFILE_GEN_C " *                      Electronics and Telecommunications Research Institute\n";
  printf WGLOBALFILE_GEN_C " * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.\n";
  printf WGLOBALFILE_GEN_C " *           No part of this software shall be reproduced, stored in a retrieval system, or\n";
  printf WGLOBALFILE_GEN_C " *           transmitted by any means, electronic, mechanical, photocopying, recording,\n";
  printf WGLOBALFILE_GEN_C " *           or otherwise, without written permission from ETRI.\n";
  printf WGLOBALFILE_GEN_C " */\n\n";
  printf WGLOBALFILE_GEN_C "#include <stdio.h>\n";
  printf WGLOBALFILE_GEN_C "#include <string.h>\n";
  printf WGLOBALFILE_GEN_C "#include <ctype.h>\n";
  printf WGLOBALFILE_GEN_C "#include <stdlib.h>\n";
  printf WGLOBALFILE_GEN_C "#include <termios.h>\n";

  printf WGLOBALFILE_GEN_C "#include \"nnTypes.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdDefines.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnStr.h\"\n";

  printf WGLOBALFILE_GEN_C "#include \"nnCmdLink.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdCommon.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdCmsh.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdNode.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdMsg.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnCmdInstall.h\"\n";
  printf WGLOBALFILE_GEN_C "#include \"nnGlobalCmd.h\"\n\n";
  #RM COMMENTS
  $line =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
  $line =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
  $line =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
  $line =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
  $gIndexFunc = 0;
  @deall = ($line =~ /(?:DENODEC|DECMD|DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@deall)
  {
    my (@deall_array);
    @deall_array = split (/,/);
    $fname = "$deall_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "extern Int32T\n$fname(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,\n";
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, \n";
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);\n";
  }

  @deall = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  if(@deall)
  {
    printf WGLOBALFILE_GEN_C "extern Int32T cmshExit(Int32T gIndex);\n";
  }
  foreach (@deall)
  {
    my (@deall_array);
    @deall_array = split (/,/);
    $fname = "$deall_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    $fnode = "$deall_array[1]";
    $fnode =~ s/^\s+//g;
    $fnode =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "static Int32T \n%s_EXIT(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,\n", $fname;
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,\n";
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)\n";
    printf WGLOBALFILE_GEN_C "{\n";
    printf WGLOBALFILE_GEN_C "    Int8T msgBuf[CMD_IPC_MAXLEN];\n";
    printf WGLOBALFILE_GEN_C "    cmdIpcMsg_T cmIpcHdr;\n";
    printf WGLOBALFILE_GEN_C "    Int32T length;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.code = CMD_IPC_CODE_FUNC_EXIT;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.mode = cmsh->modeTrace[cmsh->currentDepth].mode;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.userKey = cmsh->userKey;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.srcID = IPC_CMSH_MGR;\n";
    printf WGLOBALFILE_GEN_C "    cmIpcHdr.dstID = IPC_CMSH_MGR;\n";
    printf WGLOBALFILE_GEN_C "    cmdIpcAddHdr(msgBuf, &cmIpcHdr);\n";
    printf WGLOBALFILE_GEN_C "    cmdIpcSend(cmsh->cmfd, msgBuf);\n";
    printf WGLOBALFILE_GEN_C "    if((length = cmdIpcRecv(cmsh->cmfd, msgBuf)) < 0)\n";
    printf WGLOBALFILE_GEN_C "    {\n";
    printf WGLOBALFILE_GEN_C "        cmdPrint(cmsh, \"Can't exec command exit.\\n\");\n";
    printf WGLOBALFILE_GEN_C "        return CMD_IPC_ERROR;\n";
    printf WGLOBALFILE_GEN_C "    }\n";
    printf WGLOBALFILE_GEN_C "    else\n";
    printf WGLOBALFILE_GEN_C "    {\n";
    printf WGLOBALFILE_GEN_C "    cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);\n";
    printf WGLOBALFILE_GEN_C "    switch(cmIpcHdr.code)\n";
    printf WGLOBALFILE_GEN_C "    {\n";
    printf WGLOBALFILE_GEN_C "      case CMD_IPC_CODE_AUTH_ENABLE_RES_OK:\n";
    printf WGLOBALFILE_GEN_C "      {\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->currentDepth--;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prompt = cmsh->ctree->prompt[cmsh->modeTrace[cmsh->currentDepth].mode];\n";
    printf WGLOBALFILE_GEN_C "      }\n";
    printf WGLOBALFILE_GEN_C "      break;\n";
    printf WGLOBALFILE_GEN_C "      case CMD_IPC_CODE_CLIENT_REG_RES_OK:\n";
    printf WGLOBALFILE_GEN_C "      {\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->currentDepth--;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prompt = cmsh->ctree->prompt[cmsh->modeTrace[cmsh->currentDepth].mode];\n";
    printf WGLOBALFILE_GEN_C "      }\n";
    printf WGLOBALFILE_GEN_C "      break;\n";
    printf WGLOBALFILE_GEN_C "      case CMD_IPC_CODE_FUNC_EXIT:\n";
    printf WGLOBALFILE_GEN_C "      {\n";
    printf WGLOBALFILE_GEN_C "        cmdPrint(cmsh, \"Goodbye...\\n\");\n";
    printf WGLOBALFILE_GEN_C "        cmshExit(0);\n";
    printf WGLOBALFILE_GEN_C "      }\n";
    printf WGLOBALFILE_GEN_C "      break;\n";
    printf WGLOBALFILE_GEN_C "    }\n";
    printf WGLOBALFILE_GEN_C "    cmdUpdateMode(cmsh, cargc, cargv);\n";
    printf WGLOBALFILE_GEN_C "    }\n";
    printf WGLOBALFILE_GEN_C "    return (CMD_IPC_OK);\n";
    printf WGLOBALFILE_GEN_C "}\n";
    printf WGLOBALFILE_GEN_C "static Int32T \n%s_CLI(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,\n", $fname;
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,\n";
    printf WGLOBALFILE_GEN_C "\t\t\t\tInt32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)\n";
    printf WGLOBALFILE_GEN_C "{\n";
    printf WGLOBALFILE_GEN_C "  Int8T *tokenStr, *tmpStr = cmsh->inputStr;\n";
    printf WGLOBALFILE_GEN_C "  Int32T flags;\n";
    printf WGLOBALFILE_GEN_C "  enum cmdToken type;\n";
    printf WGLOBALFILE_GEN_C "  cmsh->modeTrace[cmsh->currentDepth].modeArgc = 0;\n";
    printf WGLOBALFILE_GEN_C "  if(cmsh->ctree->nodePrivilege[cmsh->modeTrace[cmsh->currentDepth].mode] < cmsh->ctree->nodePrivilege[cmsh->cel->nextNode])\n";
    printf WGLOBALFILE_GEN_C "  {\n";
    printf WGLOBALFILE_GEN_C "      while((tmpStr = cmdGetToken(tmpStr, &type, &flags, &tokenStr)))\n";
    printf WGLOBALFILE_GEN_C "      {\n";
    printf WGLOBALFILE_GEN_C "          cmsh->modeTrace[cmsh->currentDepth].modeArgv[cmsh->modeTrace[cmsh->currentDepth].modeArgc] = strdup(tokenStr);\n";
    printf WGLOBALFILE_GEN_C "          cmsh->modeTrace[cmsh->currentDepth].modeArgc++;\n";
    printf WGLOBALFILE_GEN_C "      }\n";
    #DENODEC
    $gIndex = 0;
    foreach (@ARGV)
    {
      $fileNode = $_;
      open (PTR, "< $fileNode");
      local $/; undef $/;
      $lineNode = <PTR>;
      close (PTR);
      $fileNode =~ s{.*/}{};      # removes path
      $fileNode =~ s{\.[^.]+$}{}; # removes extension
      $ucFileNode = uc $fileNode;
      $gIndexFunc = 0;
      $lineNode =~s/^#if 0\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\2/gsm;  # Remove #if 0
      $lineNode =~s/^#if 1\n(.*?\n)(?:#else\n(.*?\n))?#endif\n/\1/gsm;  # Remove #else of #if 1
      $lineNode =~ s/\/\/[^\n\r]*(\n\r)?//g;                            # Remove // Comments
      $lineNode =~ s/\/\*+([^*]|\*(?!\/))*\*+\///g;                     # Remove /**/ Comments
      @denodc = ($lineNode =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
      foreach (@denodc)
      {
        my (@denodc_array);
        @denodc_array = split (/,/);
        $denodc_fname = "$denodc_array[0]";
        $denodc_fname =~ s/^\s+//g;
        $denodc_fname =~ s/\s+$//g;
        $denodc_fnode = "$denodc_array[1]";
        $denodc_fnode =~ s/^\s+//g;
        $denodc_fnode =~ s/\s+$//g;
        $denodc_comps = "$denodc_array[2]";
        $denodc_comps =~ s/^\s+//g;
        $denodc_comps =~ s/\s+$//g;
        if($fnode eq $denodc_fnode)
        {
          printf WGLOBALFILE_GEN_C "      if(cmdChangeNodeReq(cmsh, $denodc_fnode, CMD_FUNC_".$ucFileNode."_".$gIndexFunc."_ID, $denodc_comps, cargc, cargv) != CMD_IPC_OK)\n";
          printf WGLOBALFILE_GEN_C "      {\n";
          printf WGLOBALFILE_GEN_C "        return CMD_IPC_OK;\n";
          printf WGLOBALFILE_GEN_C "      }\n";
        }
        $gIndexFunc++;
      }
      $gIndex++;
    }
    #END
    printf WGLOBALFILE_GEN_C "      if(%s(cmsh, uargc1, uargv1, uargc2, uargv2, uargc3, uargv3, uargc4, uargv4, uargc5, uargv5, cargc, cargv) == CMD_IPC_OK)\n", $fname;
    printf WGLOBALFILE_GEN_C "      {\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->currentDepth++;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;\n";
    printf WGLOBALFILE_GEN_C "        cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];\n";
    printf WGLOBALFILE_GEN_C "      }\n";
    printf WGLOBALFILE_GEN_C "  }\n";
    printf WGLOBALFILE_GEN_C "  cmdUpdateMode(cmsh, cargc, cargv);\n";
    printf WGLOBALFILE_GEN_C "  return CMD_IPC_OK;\n";
    printf WGLOBALFILE_GEN_C "}\n";
  }
  printf WGLOBALFILE_GEN_C "void\n";
  printf WGLOBALFILE_GEN_C "cmdFuncGlobalInstall(struct cmsh *cmsh)\n";
  printf WGLOBALFILE_GEN_C "{\n";
  $gIndexFunc = 0;
  @deall = ($line =~ /(?:DENODEC)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@deall)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fname = "$defun_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "\tcmdFuncInstall(cmsh, CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID, *$fname);\n";
    $gIndexFunc++;
  }
  @deall = ($line =~ /(?:DECMD)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@deall)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fname = "$defun_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "\tcmdFuncInstall(cmsh, CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID, *$fname);\n";
    $gIndexFunc++;
  }
  @deall = ($line =~ /(?:ALICMD)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@deall)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fname = "$defun_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "\tcmdFuncInstall(cmsh, CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID, *$fname);\n";
    $gIndexFunc++;
  }
  @deall = ($line =~ /(?:DENODE)\s*\((.+?)\);?\s?\s?\n/sg);
  foreach (@deall)
  {
    my (@defun_array);
    @defun_array = split (/,/);
    $fname = "$defun_array[0]";
    $fname =~ s/^\s+//g;
    $fname =~ s/\s+$//g;
    printf WGLOBALFILE_GEN_C "\tcmdFuncInstall(cmsh, CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID, *%s_CLI);\n",$fname;
    $gIndexFunc++;
    printf WGLOBALFILE_GEN_C "\tcmdFuncInstall(cmsh, CMD_FUNC_".$ucFile."_".$gIndexFunc."_ID, *%s_EXIT);\n",$fname;
    $gIndexFunc++;
  }
  printf WGLOBALFILE_GEN_C "}\n";
  close(WGLOBALFILE_GEN_C);
  $index++;
}
