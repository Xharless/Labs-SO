#variables
SOURCE_DIR := .
BUILD_DIR := build
MAIN_CLASS := Lab3

all: compile run

#compilar los archivos
compile:
	mkdir -p $(BUILD_DIR)
	javac -d $(BUILD_DIR) $(SOURCE_DIR)/*.java

#ejecutar
run:
	java -cp $(BUILD_DIR) $(MAIN_CLASS)

#limpiar los archivos compilados
clean:
	rm -rf $(BUILD_DIR)