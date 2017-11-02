# wigReg
**Wig**gle **Reg**ulator is a fast, memory-undemanding software to regulate wig-files after [MACS](http://liulab.dfci.harvard.edu/MACS/00README.html) and [PeakRanger](http://ranger.sourceforge.net/manual1.18.html).<br>
See [Problem statement](#problem-statement).

The program runs on the command line under Linux and Windows, and can be used in pipe-line.

## Installation
### Executable file

**Linux**<br>
Go to the desire directory and type commands:<br>
```wget -O wigReg.gz https://github.com/fnaumenko/wigReg/releases/download/1.0/wigReg-Linux-x64.gz```<br>
```gzip -d wigReg.gz```<br>
```chmod +x wigReg```

**Windows**<br>
Download archive from [here](https://github.com/fnaumenko/wigReg/releases/download/1.0/wigReg-Windows-x64.zip) 
and unzip by any archiver, for instance [WinRar](https://www.win-rar.com/download.html?&L=0).

### Compiling in Linux
Required libraries:<br>
g++<br>
zlib (optionally)

Go to the desired directory and type commands:<br>
```wget -O wigReg.zip https://github.com/fnaumenko/wigReg/archive/1.0.zip```<br>
```unzip wigReg.zip```<br>
```cd wigReg-1.0```<br>
```make```

If **zlib** is not installed on your system, a linker message will be displayed.<br>
In that case you can compile the program without the ability to work with .gz files: 
open *makefile* in any text editor, uncomment last macro in the second line, comment third line, save *makefile*, and try ```make``` again.<br>
To be sure about **zlib** on your system, type ```whereis zlib```.

## Usage
```
wigReg [options] input.wig stdout|output.wig
```
### Help
```
Options:
  -p <PR|MACS|AUTO>     program-source generated input wiggle:
                        PeakRanger, MACS or AUTOdetection [AUTO]
  -f|--frag-len <int>   length of fragment. Ignored for the wiggle from MACS [200]
  -s|--space <int>      resolution: minimal span in bp from which intervals will be saved.
                        Ignored for the wiggle from MACS [10]
  -t|--time             print run time
  -h|--help             print usage information and exit 
```

## Details

### Input

**wigReg** is designed to regulate wiggle files from MACS and PeakRanger only. 
If wiggle is generated by another program, and option ```-p``` has not ```AUTO``` value, the result is unpredictable.<br>
Only *variableStep* formatting is allowed.

Compressed files in gzip format (.gz) are acceptable.

### Output
```stdout```<br>
Redirect output to standard  output. It allows to build **wigReg** into pipe-line, for instance<br>
```        wigReg inFile.wig stdout | wigToBigWig –clip stdin chrom.sizes outFile.bw```

### Options description
Enumerable option values are case insensitive.

```-p <PR|MACS|AUTO>```<br>
Program what generates input wiggle. Lower case value is appropriate as well.<br>
**wigReg** reliably detects program-source by definition- or comment line. 
But if wiggle is restored from *bigWig*, this information is lost. 
In such case you should define program-source manually.<br>
By the way, wiggle file recovered from *bigWig* even lost required track type, which makes this wiggle invalid (it is true at least for wigToBigWig version from 06-Aug-2013).<br>
To rehabilitate it you need to put into track type element *‘track type=wiggle_0’*, f.e. by command<br>
```        echo track type=wiggle_0 | cat – inFile.wig > outFile.wig```<br>
Sure the content of wiggles from MACS and PeakRanger are definitely different. 
Why **wigReg** cannot recognise program-source any way? 
Because in general it may needed to scan uncertain number of data lines, what breaks data flow processing and reduces efficiency.<br>
Default: ```AUTO```

```-f|--frag-len <int> ```<br>
Length of fragment (reads extension) in bp, needs to extend singleton features in wiggles from PeakRanger.<br>
This option ignored for wiggles from MACS.<br>
Default: 200

```-s|--space <int>```<br>
The resolution for saving wiggle from PeakRanger. 
This value defines the minimal span in bps from which raw tags will be saved.<br>
The initial resolution of wiggle from PeakRanger is 1. 
This option allows to decrease the resolution, what can be appropriate in some circumstances. 
For instance, run time and memory requirement for the comparison wiggles process are in inverse relation with their resolution.<br>
Moreover resolution of 1 is surplus for most practical purposes.<br>
This option ignored for wiggles from MACS since their resolution defined by the program itself.<br>
Default: 10

## Problem statement
Displaying alignment’s coverage is inseparable part of studying in ChIP-sequencing. 
Though in practice the coverage is displaying via [bigWig](https://genome.ucsc.edu/goldenpath/help/bigWig.html) format, 
it is made from [wiggle](https://genome.ucsc.edu/goldenpath/help/wiggle.html).<br>

Along with the other software, two of the known peak-callers are generating this format: [MACS](http://liulab.dfci.harvard.edu/MACS/00README.html) (most popular) 
and [PeakRanger](http://ranger.sourceforge.net/manual1.18.html) (not so popular, but one of the best one).<br>
Unfortunately both of them have some issues in this aspect.

MACS generates wiggles with manageable resolution, but it creates separate data line (row tag) for each span. 
In fact it does not use the facility of *variableStep* formatting, repeating equivalent data with regular intervals like *fixedStep*. 
Eventually the output file has unjustified size, what is especially perceptibly in case of small resolution (and as a result a huge amount of data lines).
Besides wiggle generation is inherent in peak-calling process and consumes appreciable time/memory.

PeakRanger generates wiggle independently from peak-calling process and much more faster (about 100 times). 
This also allows the generation of wiggles for each strand separately.<br>
But its issue is more serious: although each its interval has unique value, they all have initial span = 1. 
Formally valid, those data are invalid in essence. 
Firstly, they look strange in genome browser by zooming, as it shows in ![figure](https://github.com/fnaumenko/wigReg/tree/master/pict/MACS-PeakRanger-comparison.png). 
Secondly, and more substantially, they cannot be used in treatment, like comparison etc.<br>
Moreover PeakRanger does not allow to manage resolution.<br>

That is true at least for MACS version less or equal 1.4.2, and for PeakRanger version less or equal 1.16.
**wigReg** utility intends to fix these issues.

## Benchmarking
The run-time and sizes measuring was performed for wiggles from MACS (for the first chromosome only) on server Intel(R) Xeon(R) CPU X5560  2.80GHz with rapid discs.

Two initial wig files from MACS have been checked: contained first chromosome (mm9) only, and for the whole genome.

1.	Size measuring

<table>
  <tr>
	<th>measure</th><th colspan="3">chrom 1</th><th colspan="3">all chroms</th>
  </tr>
  <tr align="center">
	<td align="left">space, bps</td><td>1</td><td>5</td><td>10</td><td>1</td><td>5</td><td>10</td>
  </tr>
  <tr>
	<td>size wig, Mb</td><td>1 000</td><td>200</td><td>100</td><td>13 023</td><td>2 604</td><td>1 302</td>
  </tr>
  <tr align="center">
	<td>size’s ratio wig/reg.wig</td><td>11.9</td><td>2.9</td><td>1.7</td><td>10.6</td><td>2.7</td><td>1.6</td>
  </tr>
  <tr align="center">
	<td align="left">size bw, Mb</td><td>192</td><td>48</td><td>23</td><td>2 556</td><td>632</td><td>308</td>
  </tr>
  <tr align="center">
	<td align="left">size’s ratio bw/reg.bw</td><td>1.5</td><td>0.5</td><td>0.3</td><td colspan="3">bigToBigWig error</td>
  </tr>
</table>

2.	Time measuring<br>
Unfortunately generation bw-file from regulated wig for the whole genome leads to inexplicable wigToBigWig  error *‘There's more than one value for chr… base …’* (see f.e. [FAQ post1](http://seqanswers.com/forums/showthread.php?t=4870), [FAQ post2](http://seqanswers.com/forums/showthread.php?p=193135#post193135)), so this file was excluded from benchmarking.
Since the most practical goals are to generate bigWigs,  we have measured the run time of chain<br>
```wigReg inFile.wig stdout | wigToBigWig stdin chrom.sizes outFile.bw```

   space, bps|1|5|10
----------------------------------|-------|-------|-------
wig->reg.wig by wigReg|0:10|0:03|0:01
wig->bw      by wigToBigWig|1:45|0:19|0:10
wig->reg.bw  by wigReg\|wigToBigWig|0:30|0:20|0:16
time ratio wig->bw/wig->reg.bw|3.5|1|0.6

The sizes of regulated wiggle-files are definitely reduced. 
The most impressive effect is observed by single resolution.<br>
But sizes of bw-files are not in such deterministic dependence. 
We even observe paradoxical results when smaller regulated files are compressed into bigger binary format.<br>
Though in idea the size of binary bw-file should not be dependent on wiggle representation, since there is exactly the same content.<br>
It is true for times as well. 
For single resolution the common compressing time is reduced to 3.5 times by using the chain with **wigReg**, but it has no effect for resolution of 5, and even increased in case of resolution of 10.

The only explanation we can suppose is that wigToBigWig utility is not well-optimized.
The final recommendation for MACS’s wiggles is using **wigReg** in case of resolution less than 5.
For PeakRanger’s wiggles using wigReg is irreplaceable.

##
If you face to bugs, incorrect English, or have commentary/suggestions, please do not hesitate to write me on fedor.naumenko@gmail.com
