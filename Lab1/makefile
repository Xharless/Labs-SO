
# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g

# Nombre del ejecutable
TARGET = main

# Archivos fuente y objeto
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

# Regla para construir el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regla para construir los archivos objeto
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Regla para limpiar los archivos generados
clean:
	rm -f $(TARGET) $(OBJS)
