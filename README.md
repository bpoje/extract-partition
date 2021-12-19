# Extract primary partition from flat disk image

You can create disk image using commands dd or ddrescue. Note that dd command can be dangerous (it is easy to lose data if you're not careful).

Compile:
```
[user@host test]$ gcc -o extPart extPart.c -Wall
```

Usage:
```
[user@host test]$ ./extPart 
	Usage: fileIn filePartitionOut numberOfPartition(0,1,2 or 3)
[user@host test]$
```

Check disk copy (dd) from examples:
```
[user@host test]$ sudo parted ./image1 print
	Model:  (file)
	Disk /home/ssitbp1/github/tools/testFiles/out/image1: 1074MB
	Sector size (logical/physical): 512B/512B
	Partition Table: msdos
	Disk Flags: 

	Number  Start   End     Size   Type     File system  Flags
	 1      1049kB  269MB   268MB  primary  fat16
	 2      269MB   806MB   537MB  primary  fat32        lba
	 3      806MB   941MB   134MB  primary  ntfs
	 4      941MB   1074MB  133MB  primary  ext3

[user@host test]$
```

Extract second (0 is the first partition) primary partition:
```
[user@host test]$ ./extPart image1 partition1 1
	Boot signature OK!
	Partition 0
		Starting sector: 2048
		Partition size (in sectors): 524288

	Partition 1
		Starting sector: 526336
		Partition size (in sectors): 1048576

	Partition 2
		Starting sector: 1574912
		Partition size (in sectors): 262144

	Partition 3
		Starting sector: 1837056
		Partition size (in sectors): 260096

	Selected partition: 1
	Starting Sector: 526336, Partition Size: 1048576
	Sectors transfered: 1048576
Done!
[user@host test]$
```




Check extracted partition:
```
[user@host test]$ sudo parted ./partition1 print
	Model:  (file)
	Disk /home/ssitbp1/github/tools/testFiles/out/partition1: 537MB
	Sector size (logical/physical): 512B/512B
	Partition Table: loop
	Disk Flags: 

	Number  Start  End    Size   File system  Flags
	 1      0.00B  537MB  537MB  fat32

[user@host test]$
```

Extract forth primary partition:
```
[user@host test]$ ./extPart image1 partition3 3
	Boot signature OK!
	Partition 0
		Starting sector: 2048
		Partition size (in sectors): 524288

	Partition 1
		Starting sector: 526336
		Partition size (in sectors): 1048576

	Partition 2
		Starting sector: 1574912
		Partition size (in sectors): 262144

	Partition 3
		Starting sector: 1837056
		Partition size (in sectors): 260096

	Selected partition: 3
	Starting Sector: 1837056, Partition Size: 260096
	Sectors transfered: 260096
	Done!
[user@host test]$
```

Check extracted partition:
```
[user@host test]$ sudo parted ./partition3 print
	Model:  (file)
	Disk /home/ssitbp1/github/tools/testFiles/out/partition3: 133MB
	Sector size (logical/physical): 512B/512B
	Partition Table: loop
	Disk Flags: 

	Number  Start  End    Size   File system  Flags
	 1      0.00B  133MB  133MB  ext3

[user@host test]$
```

