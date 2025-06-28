#!/bin/bash

# Número de threads
THREADS=16

# Valores de z0
z0_vals=(
  "0.0,0.0"
  "0.5,0.5"
  "0.355,0.337"
  "-0.70176,-0.3842"
  "-0.835,-0.2321"
  "-0.8,0.156"
  "0.326,0.71"
  "0.45,0.1428"
  "0.48,0.001"
  "0.37,-0.1"
)

# Valores de maxIterations
max_iter_vals=(
  10
  20
  35
  50
  70
  125
  250
  500
  1000
  2000
)

# Valores de exponent d
exponent_vals=(
  -1
  2
  3
  7
  13
  37
  50
  100
)

# Diretório base de resultados
BASE_DIR="resultados_combinados"

# Verificar se o script foi chamado com o parâmetro --julia
USE_JULIA=false
if [[ "$1" == "--julia" ]]; then
  USE_JULIA=true
  shift
fi

echo "Iniciando testes extensivos combinando z0, exponent (d) e maxIterations..."
for d in "${exponent_vals[@]}"; do
  for z0 in "${z0_vals[@]}"; do
    for max_iter in "${max_iter_vals[@]}"; do
      # Sanitizar nomes de pastas/arquivos
      z0_dir="${z0//,/_}"
      output_dir="$BASE_DIR/d_${d}/z0_${z0_dir}"
      mkdir -p "$output_dir"

      echo "d=$d | z0=($z0) | maxIterations=$max_iter"
      if $USE_JULIA; then
        ./bin/mandelbrot --threads $THREADS --julia "$z0" --maxIterations "$max_iter" --exponent "$d"  \
          > "$output_dir/output_maxIter_${max_iter}.txt"
      else
        ./bin/mandelbrot --threads $THREADS --z0 "$z0" --maxIterations "$max_iter" --exponent "$d" \
          > "$output_dir/output_maxIter_${max_iter}.txt"
      fi
    done
  done
done

echo "Todos os testes foram concluídos com sucesso."
