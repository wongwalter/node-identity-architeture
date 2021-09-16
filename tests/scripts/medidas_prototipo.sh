#!/bin/sh

# Tamanho dos pacotes a serem utilizados como parametros
packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300"
#packet_size="500 400 300"

# Tempo de cada medida (ms)
time=10000

# Taxa de transmissao de pacotes. Valor experimental 9140 sem perdas

# Numero de repeticoes por medida
repeat=10

#Medidas com o prototipo
for i in $packet_size; do
	for (( n = 0; n < $repeat; n++ )); do
		echo "UDP packet size $i com prototipo - iteracao $n"
		./ITGSend -a 1.0.0.3 -C 9140 -c $i -t $time -x udp_prot_9140_$i-r$n
	done
done
