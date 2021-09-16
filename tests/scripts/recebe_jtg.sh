#!/bin/sh

# Catch INT signal and exit
trap "exit" 1 2 3

echo "Resultados do JTG: resultado_jtg.txt"

# Protect old data file
if [ -f "resultado_jtg.txt" ]; then
	echo "Arquivo de resultados jah existente. Deseja sobreescrever? [s/n]"
	read answer
	if [ $answer = "n" ]; then
		echo "Finalizado"
		exit
	fi
fi

packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300"

for i in $packet_size; do
	for ((n=0;n<10;n++)); do
		./jtg -ru
	done
	echo ">>>>>>>>>>>>>> packet size $i"
done
