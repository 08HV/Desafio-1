/*
 * Programa demostrativo de manipulaciónprocesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 *
 * Autores: Augusto Salazar Y Aníbal Guerra
 * Fecha: 06/04/2025
 * Asistencia de ChatGPT para mejorar la forma y presentación del código fuente
 */

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <cstdlib>
#include <vector>
#include <string>



using namespace std;


unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* generateI_m(int width, int height, int seed);
unsigned char* Opera_xor(unsigned char* pixelData, unsigned char* generateI_m, int size);
unsigned char* Opera_rota(unsigned char* pixelData, int size, int n);
unsigned char* Opera_despla(unsigned char* pixelData, int size, int n, int etapa);
void Enmascaramiento(unsigned char* loadPixels, unsigned char* mascaraPixels,
int Width, int Height,int maskWidth, int maskHeight,int seed, int etapa);
void DesEnmascaramiento(unsigned char* pixelData, unsigned char* mascaraPixels,
int maskWidth, int maskHeight, int etapa);
unsigned char* Opera_xor_inverse(unsigned char* pixelData, unsigned char* generateI_m, int size);
unsigned char* Opera_rota_inverse(unsigned char* pixelData, int size, int n);
unsigned char* Opera_despla_inverse(unsigned char* pixelData, int size, int n, int etapa);


int main()
{
    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoMascara = "M.bmp";  //entrada Mascara
    QString archivoSalida = "I_D.bmp";
    QString archivoSalida2 = "I_Oi.bmp";// Imagen original por inversa

    // Variables para almacenar las dimensiones de la imagen
    int height = 0;
    int width = 0;
    // Variables para almacenar las dimensiones de la mascara
    int maskHeight =0;
    int maskWidth = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *originalPixels = loadPixels(archivoEntrada, width, height);

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *mascaraPixels = loadPixels(archivoMascara, maskWidth, maskHeight);

    int size = width * height*3;
    int numTransformaciones = 5;
    unsigned char *pixelData = originalPixels;

    for (int i = 0; i < numTransformaciones; ++i) {
        int transform = rand() % 3; // aleatoriamente un numero del 0-2
        if (transform == 0) {
            // OPERACION XOR
            unsigned char* randomImage = generateI_m(width, height, 1234+i);
            unsigned char* result= Opera_xor(pixelData, randomImage, size);
            if(pixelData != originalPixels){
                delete[] pixelData;
            }
            pixelData=result;
            delete[] randomImage;
        } else if (transform ==1){
            // OPERACION ROTACION
            int n = 2 + (rand() % 5);
            unsigned char* result = Opera_rota(pixelData, size, n);
            if (pixelData != originalPixels){
                delete[] pixelData;
            }
            pixelData = result;
        } else if (transform ==2){
            //OPERACION DESPLAZAMIENTO
            int n = 1 + (rand() % 4);
            unsigned char* result = Opera_despla(pixelData, size, n, i);
            if (pixelData != originalPixels){
                delete[] pixelData;
            }
            pixelData = result;
        }
    //PROCEDIMIENTO DE ENMASCARAMIENTO
        Enmascaramiento(pixelData, mascaraPixels, width, height, maskWidth, maskHeight, 5000+i, i);
    }

    // Exporta la imagen modificada a un nuevo archivo BMP
    bool exportI = exportImage(pixelData, width, height, archivoSalida);

    // Muestra si la exportación fue exitosa (true o false)
    cout << exportI << endl;

    // Libera la memoria usada para los píxeles
    delete[] pixelData;
    pixelData = nullptr;


    // Variables para almacenar la semilla y el número de píxeles leídos del archivo de enmascaramiento
    int seed = 0;
    int n_pixels = 0;

    // Carga los datos de enmascaramiento desde un archivo .txt (semilla + valores RGB)
    unsigned int *maskingData = loadSeedMasking("M1.txt", seed, n_pixels);

    // Muestra en consola los primeros valores RGB leídos desde el archivo de enmascaramiento
    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }

    // Libera la memoria usada para los datos de enmascaramiento
    if (maskingData != nullptr){
        delete[] maskingData;
        maskingData = nullptr;
    }

 // proceso para volver a la imagen original.

    // Carga la imagen BMP en memoria dinamica y obtiene ancho y alto
    unsigned char *pixelI_D = loadPixels(archivoSalida, maskWidth, maskHeight);
    unsigned char *pixelData1 = pixelI_D;


    vector<string> transformaciones;

    // ciclo desde la última transformación a la primera
    for (int etapa = 4; etapa >= 0; etapa--) {
        cout << "Deshaciendo etapa " << etapa << "..." << endl;

        // Desenmascaramiento por los archivos .txt
        DesEnmascaramiento(pixelData, mascaraPixels, maskWidth, maskHeight, etapa);
        cout << "- Desenmascaramiento realizado (archivo M" << etapa << ".txt)" << endl;

        // aplicar transformación inversa
        bool invertido = false;

        // desplazamientos inversos
        for (int n = 1; n <= 4 && !invertido; ++n) {
            unsigned char* intento = Opera_despla_inverse(pixelData1, size, n, etapa);
            int validos = 0;
            for (int i = 0; i < size; ++i) {
                if (intento[i] >= 32 && intento[i] <= 250) validos++;
            }
            if (validos > size * 0.80) {
                cout << "- Transformación detectada: Desplazamiento inverso (n = " << n << ")" << endl;
                transformaciones.push_back("Etapa " + to_string(etapa) + ": Desplazamiento inverso (n = " + to_string(n) + ")");
                delete[] pixelData1;
                pixelData1 = intento;
                invertido = true;
                break;
            }
            delete[] intento;
        }

        // rotaciones inversas
        for (int n = 1; n <= 7 && !invertido; ++n) {
            unsigned char* intento = Opera_rota_inverse(pixelData1, size, n);
            int validos = 0;
            for (int i = 0; i < size; ++i) {
                if (intento[i] >= 32 && intento[i] <= 250) validos++;
            }
            if (validos > size * 0.80) {
                cout << "- Transformación detectada: Rotación inversa (n = " << n << ")" << endl;
                transformaciones.push_back("Etapa " + to_string(etapa) + ": Rotación inversa (n = " + to_string(n) + ")");
                delete[] pixelData1;
                pixelData1 = intento;
                invertido = true;
                break;
            }
            delete[] intento;
        }

        //  XOR inversa
        if (!invertido) {
            unsigned char* xorr= generateI_m(width, height, 1234 + etapa);
            unsigned char* intento = Opera_xor_inverse(pixelData1, xorr, size);
            int validos = 0;
            for (int i = 0; i < size; ++i) {
                if (intento[i] >= 32 && intento[i] <= 250) validos++;
            }
            if (validos > size * 0.80) {
                cout << "- Transformación detectada: XOR inverso" << endl;
                transformaciones.push_back("Etapa " + to_string(etapa) + ": XOR inverso");
                delete[] pixelData1;
                pixelData1 = intento;
                invertido = true;
            } else {
                delete[] intento;
            }
            delete[] xorr;
        }

        if (!invertido) {
            cout << "No se pudo identificar la transformación en etapa " << etapa << endl;
            transformaciones.push_back("Etapa " + to_string(etapa) + ": No identificada");
        }
    }

    // Exportar imagen recuperada
    bool exportada = exportImage(pixelData1, width, height, archivoSalida2 );
    cout << exportI << endl;

    // Comparar con la original
    int errores = 0;
    for (int i = 0; i < size; ++i) {
        if (pixelData1[i] != originalPixels[i]){
            errores++;}
    }
    if (errores == 0) {
        cout << "Imagen reconstruida con éxito. Es igual a la original." << endl;
    } else {
        cout << "Imagen reconstruida con diferencias en " << errores << " bytes." << endl;
    }

    // Mostrar transformaciones detectadas
    cout << "\n Transformaciones identificadas (de etapa 4 a 0):" << endl;
    for (const auto& t : transformaciones) {
        cout << t << endl;
    }

    // Limpieza de memoria
    delete[] pixelData;
    pixelData = nullptr;
    delete[] originalPixels;
    originalPixels = nullptr;
    delete[] mascaraPixels;
    mascaraPixels = nullptr;

    return 0;
}


unsigned char* loadPixels(QString input, int &width, int &height){
/*
 * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
 *
 * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
 * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
 * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
 * de cada píxel de la imagen, sin rellenos (padding).
 *
 * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
 * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
 * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
 * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
 *         Devuelve nullptr si la imagen no pudo cargarse.
 *
 * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando `delete[]`.
 */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);              // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;     // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                    // Copia los píxeles RGB de esa línea (sin padding)
    }

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
/*
 * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
 *
 * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
 * `pixelData`, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
 * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
 * en formato BMP en la ruta especificada.
 *
 * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
 *                  El tamaño debe ser igual a width * height * 3 bytes.
 * @param width Ancho de la imagen en píxeles.
 * @param height Alto de la imagen en píxeles.
 * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
 *
 * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
 *
 * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
 */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }

}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels){
/*
 * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
 *
 * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
 * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
 * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
 * y finalmente carga los valores en un arreglo de enteros.
 *
 * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
 * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
 * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
 *                 (equivalente al número de líneas después de la semilla).
 *
 * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
 *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
 *
 * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
 */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        // Verificar si el archivo pudo abrirse correctamente
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla desde la primera línea del archivo
    archivo >> seed;

    int r, g, b;

    // Contar cuántos grupos de valores RGB hay en el archivo
    // Se asume que cada línea después de la semilla tiene tres valores (r, g, b)
    while (archivo >> r >> g >> b) {
        n_pixels++;  // Contamos la cantidad de píxeles
    }

    // Cerrar el archivo para volver a abrirlo desde el inicio
    archivo.close();
    archivo.open(nombreArchivo);

    // Verificar que se pudo reabrir el archivo correctamente
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }

    // Reservar memoria dinámica para guardar todos los valores RGB
    // Cada píxel tiene 3 componentes: R, G y B
    unsigned int* RGB = new unsigned int[n_pixels * 3];

    // Leer nuevamente la semilla desde el archivo (se descarta su valor porque ya se cargó antes)
    archivo >> seed;

    // Leer y almacenar los valores RGB uno por uno en el arreglo dinámico
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    // Cerrar el archivo después de terminar la lectura
    archivo.close();

    // Mostrar información de control en consola
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;

    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}

unsigned char* generateI_m(int width, int height, int seed) {
    srand(seed);
    int size = width * height * 3;
    unsigned char* randomPixels = new unsigned char[size];
    for (int i = 0; i < size; ++i) {
        randomPixels[i] = rand() % 256;
    }
    return randomPixels;
}

unsigned char* Opera_xor(unsigned char* pixelData, unsigned char* generateI_m, int size){
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        result[i] = pixelData[i] ^ generateI_m[i];
    }
    return result;
}

unsigned char* Opera_rota(unsigned char* pixelData, int size, int n){
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        result[i] = (pixelData[i] >> n) | (pixelData[i] << (8 - n)); // Rotación
    }
    return result;
}

unsigned char* Opera_despla(unsigned char* pixelData, int size, int n, int etapa){
    unsigned char* result = new unsigned char[size];

    ofstream file("bits_p"+ to_string(etapa)+".txt");

    for (int i = 0; i < size; i++) {
        unsigned char Bitsper = pixelData[i] & ((1 << n) - 1);
        file << static_cast<int>(Bitsper)<<" ";
        result[i] = pixelData[i] >> n;
    }
    file.close();
    return result;
}

void Enmascaramiento(unsigned char* pixelData, unsigned char* mascaraPixels,
    int Width, int Height,int maskWidth, int maskHeight,int seed, int etapa) {
    int Size = Width * Height * 3;
    int maskSize = maskWidth * maskHeight * 3;
    srand(seed);
    int s = rand() % (Size - maskSize);  //rango
    unsigned char* mascara = new unsigned char[maskSize];

    for (int k = 0; k < maskSize; ++k) {
        int suma = pixelData[k+s] + mascaraPixels[k];
        if (suma > 255){
            suma = 255;}
        mascara[k]=static_cast<unsigned char>(suma);

    }
    // Guardar archivo de rastreo
    ofstream file("M" + to_string(etapa) + ".txt");


    file << s << "\n";
    for (int k = 0; k < maskSize; ++k) {
        file << static_cast<int>(mascara[k]) << "\n";
    }
    file.close();

    delete[] mascara;
}

void DesEnmascaramiento(unsigned char* pixelData, unsigned char* mascaraPixels,
int maskWidth, int maskHeight, int etapa){

    int maskSize = maskWidth * maskHeight * 3;

    ifstream file("M" + to_string(etapa) + ".txt");
    int s;
    file >> s;

    unsigned char* mascara = new unsigned char[maskSize];
    for (int k = 0; k < maskSize; ++k) {
        int valor;
        file >> valor;
        mascara[k] = static_cast<unsigned char>(valor);
    }
    file.close();

    for (int k = 0; k < maskSize; ++k) {
        int resta = mascara[k] - mascaraPixels[k];
        if (resta < 0) resta = 0;
        pixelData[k + s] = static_cast<unsigned char>(resta);
    }

    delete[] mascara;
}

// Inversa de XOR
unsigned char* Opera_xor_inverse(unsigned char* pixelData, unsigned char* generateI_m, int size) {
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        result[i] = pixelData[i] ^ generateI_m[i];
    }
    return result;
}
// inversa de ROTA
unsigned char* Opera_rota_inverse(unsigned char* pixelData, int size, int n) {
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        result[i] = (pixelData[i] << n) | (pixelData[i] >> (8 - n)); // Rotación a la izquierda
    }
    return result;
}

unsigned char* Opera_despla_inverse(unsigned char* pixelData, int size, int n, int etapa) {
    unsigned char* result = new unsigned char[size];

    ifstream file("bits_p" + to_string(etapa) + ".txt");

    for (int i = 0; i < size; i++) {
        int Bitsper;
        file >> Bitsper;
        result[i] = (pixelData[i] << n) | (Bitsper & ((1 << n) - 1));
    }

    file.close();
    return result;
}





