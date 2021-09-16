#!/bin/sh

packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300 200 100"

echo "Planilha com prototipo."
echo "" > planilha_prototipo.txt

for i in $packet_size; do
	for (( n = 0; n < 10; n++ )); do
	        if [ -f udp_prot_9140_$i-r$n ]; then
        	        echo "Decodificando udp_prot_9140_$i-r$n"
                	./ITGDec udp_prot_9140_$i-r$n >> planilha_prototipo.txt
		else
			echo "Arquivo udp_prot_9140_$i-r$n nao encontrado"
        	fi
	done
done
