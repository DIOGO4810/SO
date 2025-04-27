#!/bin/bash

outputFile="scripts/tempos.csv"

# Cabeçalho do CSV
echo "comando,real,user,sys" > "$outputFile"

measure_time() {
    comando=$1
    descricao=$2

    output=$($comando)

    # Imprime a saída do comando no terminal
    echo "Resultado do comando: $comando"
    echo "$output"
    echo "|||||||||||||||||||||||||||||||||||||||"
    echo

    # Captura os tempos sem redirecionar a saída normal do comando
    tempo=$(/usr/bin/time -f "%e,%U,%S" $comando 2>&1>/dev/null)

    
    # Grava os tempos no CSV
    echo "$descricao,    $tempo" >> "$outputFile"
}

# Not Paralel
measure_time "bin/dclient -s $1" "bin/dclient -s \"$1\""
#Paralel
measure_time "bin/dclient -s $1 $2" "bin/dclient -s \"$1\" $2"
measure_time "bin/dclient -s $1 $(( $2 * 2 ))" "bin/dclient -s \"$1\" $(( $2 * 2 ))"
measure_time "bin/dclient -s $1 $(( $2 * 3 ))" "bin/dclient -s \"$1\" $(( $2 * 3 ))"
measure_time "bin/dclient -s $1 $(( $2 * 4 ))" "bin/dclient -s \"$1\" $(( $2 * 4 ))"
measure_time "bin/dclient -s $1 $(( $2 * 5 ))" "bin/dclient -s \"$1\" $(( $2 * 5 ))"
measure_time "bin/dclient -s $1 $(( $2 * 6 ))" "bin/dclient -s \"$1\" $(( $2 * 6 ))"
measure_time "bin/dclient -s $1 $(( $2 * 7 ))" "bin/dclient -s \"$1\" $(( $2 * 7 ))"
