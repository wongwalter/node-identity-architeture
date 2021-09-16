#!/bin/sh

packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300 200 100"

for i in $packet_size; do
	echo "Decodificando o arquivo udp_sem_9140_$i ..."
	./ITGDec udp_sem_9140_$i -d 100 -j 100 -b 100
	echo "Gerando o arquivo bitrate_udp_sem_9140_$i.dat para udp_sem_9140_$i"
	mv bitrate.dat bitrate_udp_sem_9140_$i
	echo "Gerando o arquivo jitter_udp_sem_9140_$i.dat para udp_sem_9140_$i"
	mv jitter.dat jitter_udp_sem_9140_$i
	echo "Gerando o arquivo delay_udp_sem_9140_$i.dat para udp_sem_9140_$i"
	mv delay.dat delay_udp_sem_9140_$i
done

for i in $packet_size; do
	echo "Decodificando o arquivo udp_com_9140_$i ..."
	./ITGDec udp_com_9140_$i -d 100 -j 100 -b 100
	echo "Gerando o arquivo bitrate_udp_com_9140_$i.dat para udp_com_9140_$i"
	mv bitrate.dat bitrate_udp_com_9140_$i
	echo "Gerando o arquivo jitter_udp_com_9140_$i.dat para udp_com_9140_$i"
	mv jitter.dat jitter_udp_com_9140_$i
	echo "Gerando o arquivo delay_udp_com_9140_$i.dat para udp_com_9140_$i"
	mv delay.dat delay_udp_com_9140_$i
done
