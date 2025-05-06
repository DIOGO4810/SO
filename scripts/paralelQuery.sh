#!/bin/bash

outputFile="scripts/tempos.csv"

# Cabeçalho do CSV
echo "comando,real,user,sys" > "$outputFile"

measure_time() {
    comando=$1
    descricao=$2

    # Executa o comando e mede o tempo, capturando stdout e tempos
    tempo_output=$( { /usr/bin/time -f "%e,%U,%S" bash -c "$comando"; } 2>&1 )
    
    # Separa tempo da saída
    tempo=$(echo "$tempo_output" | tail -n 1)
    output=$(echo "$tempo_output" | head -n -1)

    # Mostra o output do comando
    echo "Resultado do comando: $comando"
    echo "$output"
    echo "|||||||||||||||||||||||||||||||||||||||"
    echo

    # Grava no CSV
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
