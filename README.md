# Esteganografia
Trabajo Practico para Criptografía y Seguridad

## Configurar entorno
Antes de compilar el proyecto, debe instalar las siguientes dependencias
```bash
sudo apt update
sudo apt install gcc
sudo apt install libssl-dev
```

## Compilacion
Para compilar el proyecto, basta con ejecutar
```bash
make all
```
A su vez, se pueden eliminar los binarios con el comando
```bash
make clean
```

## Ejecucion
Puede ejecutar el proyecto con el comando
```bash
./bin/stegobmp {flags}
```
Recuerde reemplazar `{flags}` con las flags que desee para indicarle al programa lo que tiene que hacer