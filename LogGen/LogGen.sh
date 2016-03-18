#!/bin/bash
if [ $# -eq 0 ]; then
	#命令行
	echo "useage : filename 骰子君 DNDRiver name3 name4 ..."
else
	#把回车处理成<br/>
	sed ':a;N;$!ba;s/\n/<br\/>/g' $1 > /cygdrive/c/temp.txt
	index=0
	alluser=""
	for args in $@
	do
		if [ $index -ne 0 ]; then
			alluser="$alluser\|$args"
		fi
		((index++))
	done
	alluser=${alluser:2}
	
	#处理/me
	sed 's/<br\/>\('${alluser}'\)<br\/>\([0-9]*:[0-9]*\):[0-9]*<br\/>\/me/\r\n\[\2\] * \1/g' /cygdrive/c/temp.txt > /cygdrive/c/temp1.txt
	#处理普通
	sed 's/<br\/>\('${alluser}'\)<br\/>\([0-9]*:[0-9]*\):[0-9]*<br\/>/\r\n\[\2\] <\1> /g' /cygdrive/c/temp1.txt > /cygdrive/c/temp.txt
	#处理小时一位前补0
	sed 's/^\[\([0-9]\):/\[0\1:/g' /cygdrive/c/temp.txt > /cygdrive/c/temp1.txt
	
	#特殊处理第一位骰子君
	#[9:45] <骰子君> 09:45:15 DNDRiver 投掷  1D4-1 = 2-1 = 1
	#[9:45] * DNDRiver 投掷  1D4-1 = 2-1 = 1
	sed 's/^\(\[[0-9]*:[0-9]*\] \)<'$2'> [0-9]*:[0-9]*:[0-9]* \(.*\) 投掷/\1\* \2 投掷/g' /cygdrive/c/temp1.txt > /cygdrive/c/temp.txt

	#去除空行
	sed ':a;s/<br\/><br\/>/<br\/>/g;ta' /cygdrive/c/temp.txt > /cygdrive/c/temp1.txt
	
	#每个换行都补上头
	sed ':a;s/^\(\[[0-9]*:[0-9]*\]\) <\('${alluser}'\)> \(.*\)<br\/>/\1 <\2> \3\r\n\1 <\2> /g;ta' /cygdrive/c/temp1.txt
	#sed ':a;N;$!ba;s/^\(\[[0-9]*:[0-9]*\] <'${alluser}'> \)\(.*\)<br\/>\(.*\)$/\1\2\r\n\1\3/g' /cygdrive/c/temp.txt
fi