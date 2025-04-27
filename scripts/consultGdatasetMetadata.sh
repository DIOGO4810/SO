#!/bin/bash


# Lê o conteúdo binário do ficheiro 'ordem' e converte para um inteiro
numLinhas=$(dd if=ordem bs=4 count=1 2>/dev/null | od -An -tu4)
echo "Número de linhas lido: $numLinhas"

# Agora o resto do script
for ((i = 1; i <= $1; i++))
do
    random_number=$(shuf -i 0-"$numLinhas" -n 1)
    bin/dclient -c "$random_number"
done
