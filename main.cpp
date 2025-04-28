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
#include <QRandomGenerator>




using namespace std;


unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
unsigned char* generateI_m(int width, int height, int seed);
unsigned char* Opera_xor(unsigned char* pixelData, unsigned char* generateI_m, int size);
unsigned char* Opera_rota(unsigned char* pixelData, int size, int n, int etapa);
unsigned char* Opera_despla(unsigned char* pixelData, int size, int n, int etapa);
void Enmascaramiento(unsigned char* loadPixels, unsigned char* mascaraPixels,
int Width, int Height,int maskWidth, int maskHeight,int seed, int etapa);
void DesEnmascaramiento(unsigned char* pixelData1, unsigned char* mascaraPixels,
int maskWidth, int maskHeight, int etapa);
unsigned char* Opera_xor_inverse(unsigned char* pixelData1, unsigned char* generateI_m, int size);
unsigned char* Opera_rota_inverse(unsigned char* pixelData1, int size, int n, int etapa);
unsigned char* Opera_despla_inverse(unsigned char* pixelData1, int size, int n, int etapa);
bool verificarEnmascaramientoEtapa(unsigned char* pixelData, unsigned char* mascaraPixels,
int Width, int Height, int maskWidth, int maskHeight, int etapa);



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
        int transform = QRandomGenerator::global()->bounded(3); // aleatoriamente un numero del 0-2
        if (transform == 0) {
            int seed = 1234 + i;
            unsigned char* randomImage = generateI_m(width, height, seed);
            ofstream file("xor_etapa" + to_string(i) + ".txt");
            if (file.is_open()) {
                file << seed;
                file.close();
            } else {
                cerr << "No se guardo la semilla XOR para etapa " << i << endl;
            }
             // OPERACION XOR
            unsigned char* result= Opera_xor(pixelData, randomImage, size);
            if(pixelData != originalPixels){
                delete[] pixelData;
            }
            pixelData=result;
            delete[] randomImage;
        }else if (transform ==1){
            // OPERACION ROTACION
            int n = 2 + (rand() % 5);
            unsigned char* result = Opera_rota(pixelData, size, n, i);
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

     // VERIFICACION DEL ENMASCARAMIENTO
        bool verificado = verificarEnmascaramientoEtapa(pixelData, mascaraPixels, width, height, maskWidth, maskHeight, i);
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
    unsigned char *pixelI_D = loadPixels(archivoSalida, width, height);
    unsigned char *pixelData1 = new unsigned char[size];
    memcpy(pixelData1, pixelI_D, size);
    delete[] pixelI_D;
    pixelI_D = nullptr;

    vector<string> transformaciones;

    // ciclo desde la última transformación a la primera
    for (int etapa = 4; etapa >= 0; etapa--) {
        cout << "Deshaciendo etapa " << etapa << "..." << endl;

        // Desenmascaramiento por los archivos .txt
        DesEnmascaramiento(pixelData1, mascaraPixels, maskWidth, maskHeight, etapa);
        cout << "- Desenmascaramiento realizado (archivo M" << etapa << ".txt)" << endl;

        // aplicar transformación inversa
        bool invertido = false;

        string archivoBits = "bits_p" + to_string(etapa) + ".txt";
        ifstream testFile(archivoBits);
        if (testFile.is_open()) {
        // desplazamientos inversos
        for (int n = 1; n <= 4 && !invertido; ++n) {
            unsigned char* intento = Opera_despla_inverse(pixelData1, size, n, etapa);
            int validos = 0;
            for (int i = 0; i < size; ++i) {
                if (intento[i] >= 10 && intento[i] <= 245) validos++;
            }
            if (validos > size * 0.80) {
                cout << "- Desplazamiento inverso (n = " << n << ")" << endl;
                transformaciones.push_back("Etapa " + to_string(etapa) + ": Desplazamiento inverso (n = " + to_string(n) + ")");
                delete[] pixelData1;
                pixelData1 = intento;
                invertido = true;
                break;
            }
            delete[] intento;
        }} else{
            cout << "no hubo transfromacion desplazamiento en esta etapa "<<etapa  << endl;
        }

        // rotaciones inversas
        if (!invertido) {
            string archivoRot = "rota_N" + to_string(etapa) + ".txt";
            ifstream rotFile(archivoRot);
            if (rotFile.is_open()) {
                int n;
                rotFile >> n;
                rotFile.close();
                // rotaciones inversas
                unsigned char* intento = Opera_rota_inverse(pixelData1, size, n, etapa);
                int validos = 0;
                for (int i = 0; i < size; ++i) {
                    if (intento[i] >= 10 && intento[i] <= 245) validos++;
                }
                if (validos > size * 0.80) {
                    cout << "Rotación inversa (n = " << n << ")" << endl;
                    transformaciones.push_back("Etapa " + to_string(etapa) + ": Rotación inversa (n = " + to_string(n) + ")");
                    delete[] pixelData1;
                    pixelData1 = intento;
                    invertido = true;
                } else {
                    delete[] intento;
                }
            } else {
                cout << "No hubo archivo de rotación rota_N" << etapa << ".txt para esta etapa." << endl;
            }
        }

        //  XOR inversa
        if (!invertido) {
            string archivosem = "xor_etapa" + to_string(etapa) + ".txt";
            ifstream semilla(archivosem);
            if (semilla.is_open()) {
                int seed;
                semilla >> seed;
                semilla.close();
            unsigned char* xorr= generateI_m(width, height, seed);
            unsigned char* intento = Opera_xor_inverse(pixelData1, xorr, size);
            int validos = 0;
            for (int i = 0; i < size; ++i) {
                if (intento[i] >= 20 && intento[i] <= 250) validos++;
            }
            if (validos > size * 0.75) {
                cout << "- XOR inverso" << endl;
                transformaciones.push_back("Etapa " + to_string(etapa) + ": XOR inverso");
                delete[] pixelData1;
                pixelData1 = intento;
                invertido = true;
            } else {
                delete[] intento;
            }
            delete[] xorr;
        }else {
                cout << "No se encontró la semilla para etapa " << etapa << ". No se aplicó XOR inverso." << endl;
            }
        }


        if (!invertido) {
            cout << "No se pudo identificar la transformación en etapa " << etapa << endl;
            transformaciones.push_back("Etapa " + to_string(etapa) + ": No identificada");
        }
    }

    // Exportar imagen recuperada
    bool exportada = exportImage(pixelData1, width, height, archivoSalida2 );
    cout << exportada << endl;

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
    cout << "Transformaciones identificadas (de etapa 4 a 0):" << endl;
    for (const auto& t : transformaciones) {
        cout << t << endl;
    }

    // Limpieza de memoria
    delete[] pixelData1;
    pixelData = nullptr;
    delete[] originalPixels;
    originalPixels = nullptr;
    delete[] mascaraPixels;
    mascaraPixels = nullptr;

    return 0;
}


unsigned char* loadPixels(QString input, int &width, int &height){

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

unsigned char* Opera_rota(unsigned char* pixelData, int size, int n, int etapa){
    unsigned char* result = new unsigned char[size];
    ofstream file("rota_N" + to_string(etapa) + ".txt");
    if (file.is_open()) {
        file << n << endl;
        file.close();
    } else {
        cerr << "Error al abrir rota_N" << etapa << endl;
    }
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

void DesEnmascaramiento(unsigned char* pixelData1, unsigned char* mascaraPixels,
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
        pixelData1[k + s] = static_cast<unsigned char>(resta);
    }

    delete[] mascara;
}

// Inversa de XOR
unsigned char* Opera_xor_inverse(unsigned char* pixelData1, unsigned char* generateI_m, int size) {
    unsigned char* result = new unsigned char[size];
    for (int i = 0; i < size; i++) {
        result[i] = pixelData1[i] ^ generateI_m[i];
    }
    return result;
}
// inversa de ROTA
unsigned char* Opera_rota_inverse(unsigned char* pixelData1, int size, int n, int etapa) {
    unsigned char* result = new unsigned char[size];

    ifstream file("rota_N" + to_string(etapa) + ".txt");
    if(!file.is_open()){
        cerr << "No se pudo abrir el archivo rota_N" << etapa << ".txt" << endl;
        return result;
    }
    for (int i = 0; i < size; i++) {
        result[i] = (pixelData1[i] << n) | (pixelData1[i] >> (8 - n)); // Rotación a la izquierda
    }
    file.close();
    return result;
}

unsigned char* Opera_despla_inverse(unsigned char* pixelData1, int size, int n, int etapa) {
    unsigned char* result = new unsigned char[size];

    ifstream file("bits_p" + to_string(etapa) + ".txt");
    if(!file.is_open()){
        cerr << "No se pudo abrir el archivo bits_p" << etapa << ".txt" << endl;
        return result;
    }

    for (int i = 0; i < size; i++) {
        int Bitsper;
        file >> Bitsper;
        result[i] = (pixelData1[i] >> n) | ((Bitsper & ((1 << n) - 1)) << (8-n));
    }

    file.close();
    return result;
}

bool verificarEnmascaramientoEtapa(unsigned char* pixelData, unsigned char* mascaraPixels,
int Width, int Height, int maskWidth, int maskHeight, int etapa) {
    int size = Width * Height * 3;
    int maskSize = maskWidth * maskHeight * 3;

    ifstream file("M" + to_string(etapa) + ".txt");
    if (!file.is_open()) {
        cout << "No se pudo abrir el archivo M" << etapa << ".txt" << endl;
        return false;
    }
    int s;
    file >> s;

    bool exito = true;

    for (int k = 0; k < maskSize; ++k) {
        int valorGuardado;
        if (!(file >> valorGuardado)) {
            cout << "Error leyendo valor en posición " << k << endl;
            exito = false;
            break;
        }

        int suma = pixelData[k + s] + mascaraPixels[k];
        if (suma > 255) {
            suma = 255;
        }

        if (suma != valorGuardado) {
            cout << "Diferencia en píxel " << k << ": esperado " << valorGuardado << ", calculado " << suma << endl;
            exito = false;
        }
    }

    file.close();

    if (exito) {
        cout << "Enmascaramiento verificado correctamente en etapa " << etapa << endl;
    } else {
        cout << "Error de enmascaramiento en etapa " << etapa << endl;
    }

    return exito;
}




