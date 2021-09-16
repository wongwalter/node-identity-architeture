#!/bin/sh

packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300 200 100"

echo "Planilha sem o prototipo."
echo "" > planilha_sem_prot.txt
for i in $packet_size; do
	if [ -f udp_sem_9140_$i ]; then
		echo "Decodificando udp_sem_9140_$i"
		./ITGDec udp_sem_9140_$i >> planilha_sem_prot.txt
	fi
done

echo "Planilha com o prototipo sem seguranca"
echo "" > planilha_prot.txt
for i in $packet_size; do
	if [ -f udp_com_9140_$i ]; then
	        echo "Decodificando udp_com_9140_$i"
        	./ITGDec udp_com_9140_$i >> planilha_prot.txt
	fi
done

echo "Planilha com o prototipo AH"
echo "" > planilha_prot_AH.txt
for i in $packet_size; do
	if [ -f udp_com_9140_AH_$i ]; then
	        echo "Decodificando udp_com_9140_AH_$i"
        	./ITGDec udp_com_9140_AH_$i >> planilha_prot_AH.txt
	fi
done

echo "Planilha com o prototipo ESP"
echo "" > planilha_prot_ESP.txt
for i in $packet_size; do
	if [ -f udp_prot_ESP_9140_$i ]; then
	        echo "Decodificando udp_prot_ESP_9140_$i"
        	./ITGDec udp_prot_ESP_9140_$i >> planilha_prot_ESP.txt
	fi
done

echo "Planilha com o prototipo full"
echo "" > planilha_prot_full.txt
for i in $packet_size; do
        if [ -f udp_prot_full_9140_$i ]; then
                echo "Decodificando udp_prot_full_9140_$i"
                ./ITGDec udp_prot_full_9140_$i >> planilha_prot_full.txt
        fi
done

