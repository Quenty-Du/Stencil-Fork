#include <Functions.h>
#include <Class_Kernel.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>


cv::Mat convolutionApply(const Kernel& kernel, cv::Mat& image)
{
    if (kernel.getSize() > image.rows || kernel.getSize() > image.cols) {
        throw std::invalid_argument("Kernel is larger than image");
    }

    int result_rows = image.rows - kernel.getSize() + 1;
    int result_cols = image.cols - kernel.getSize() + 1;
    cv::Mat result(result_rows, result_cols, CV_32FC3, cv::Scalar(0, 0, 0));

    // Проход по каналам
    for (int channel = 0; channel < 3; ++channel)
    {
        // Проход по пикселям изображения (с учетом границ размера ядра)
        for (int result_y = 0; result_y < result_rows; ++result_y)
        {
            for (int result_x = 0; result_x < result_cols; ++result_x)
            {
                // Операция свертки
                for (int kernel_y = 0; kernel_y < kernel.getSize(); ++kernel_y)
                {
                    for (int kernel_x = 0; kernel_x < kernel.getSize(); ++kernel_x)
                    {
                        result.ptr<cv::Vec3f>(result_y)[result_x][channel] += 
                        image.ptr<cv::Vec3f>(result_y + kernel_y)[result_x + kernel_x][channel] 
                        * kernel.getElement(kernel_x, kernel_y);

                    }
                }
            }
        }
    }

    return result;
}


std::vector<float> convertToFloat(const std::vector<uint8_t>& input) {
    size_t length = input.size();
    std::vector<float> floatData(length);

    for (size_t i = 0; i < length; i++) {
        floatData[i] = static_cast<float>(input[i]);
    }

    return floatData;
}


std::vector<uint8_t> convertToChar(const std::vector<float>& input) {
    size_t length = input.size();
    std::vector<uint8_t> charData(length);

    for (size_t i = 0; i < length; i++) {
        charData[i] = 
        static_cast<uint8_t>(std::clamp(std::round(input[i]), 0.0f, 255.0f));
    }

    return charData;
}


void normalize(std::vector<float>& input) {
    if (input.empty()) return;

    size_t length = input.size();

    auto [minIt, maxIt] = std::minmax_element(input.begin(), input.end());

    float maxVal = *maxIt;
    float minVal = *minIt;

    float range = maxVal - minVal;

    if (range == 0.0f){
        std::fill(input.begin(), input.end(), std::clamp(minVal, 0.0f, 255.0f));
        return;
    }

    for (size_t i = 0; i < length; i++) {
        input[i] = std::clamp((input[i] - minVal) * 255.0f / range, 0.0f, 255.0f);
    }

    return;
}


cv::Mat Sobel_operator(cv::Mat& image)
{
    Kernel horizontal (3);
    Kernel vertical (3);

    float h[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    float v[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

    horizontal.fillKernel(3, h);
    vertical.fillKernel(3, v);

    cv::Mat result_h = convolutionApply(horizontal, image);
    cv::Mat result_v = convolutionApply(vertical, image);

    cv::Mat result(result_h.rows, result_h.cols, CV_32FC3, cv::Scalar(0, 0, 0));

    for (int channel = 0; channel < 3; ++channel)
    {
        for (int y = 0; y < result.rows; ++y)
        {
            for (int x = 0; x < result.cols; ++x)
            {
                result.ptr<cv::Vec3f>(y)[x][channel] = 
                sqrt(pow(result_h.ptr<cv::Vec3f>(y)[x][channel], 2) + 
                pow(result_v.ptr<cv::Vec3f>(y)[x][channel], 2));
            }
        }
    }

    return result;
}


bool loadBMP(const std::string& filename, std::vector<uint8_t>& bgrData, 
    int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return false;
    }

    BMPFileHeader fileHeader;

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));

    if (fileHeader.bfType != 0x4D42) {
        std::cerr << "Ошибка: формат файла не является BMP" << std::endl;
        return false;
    }

    BMPInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    if (infoHeader.biSize != 40) {
        std::cerr << "Ошибка: ридер поддерживает только стандартный формат BMP, размер заголовка которого 40 байт" << std::endl;
        return false;
    }

    if (infoHeader.biBitCount != 24) {
        std::cerr << "Ошибка: поддерживаются только 24-битные BMP (3 слоя: R, G, B, по 8 бит на каждый)" << std::endl;
        return false;
    }

    if (infoHeader.biCompression != 0) {
        std::cerr << "Ошибка: ридер не поддерживает сжатые файлы" << std::endl;
        return false;
    }

    width = infoHeader.biWidth;

    height = (infoHeader.biHeight > 0) ? infoHeader.biHeight : -infoHeader.biHeight;

    bool bottomUp = (infoHeader.biHeight > 0);

    int bytesPerRow = (width * 3 + 3) & ~3;

    file.seekg(fileHeader.bfOffBits, std::ios::beg);

    bgrData.resize(width * height * 3);

    std::vector<uint8_t> rowBuffer(bytesPerRow);

    for (int y = 0; y < height; y++) {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), bytesPerRow);

        if (!file) {
            std::cerr << "Ошибка: не удалось прочитать данные" << std::endl;
            return false;
        }

        int destRow;
        if (bottomUp) {
            destRow = height - 1 - y;
        }
        else {
            destRow = y;
        }

        for (int x = 0; x < width; x++) {
            int destIndex = (destRow * width + x) * 3;

            bgrData[destIndex] = rowBuffer[x * 3];
            bgrData[destIndex + 1] = rowBuffer[x * 3 + 1];
            bgrData[destIndex + 2] = rowBuffer[x * 3 + 2];
        }
    }

    file.close();
    return true;
}


bool saveBMP(const std::string& filename, const std::vector<uint8_t>& bgrData, int width, int height) {
    int bytesPerRow = (width * 3 + 3) & ~3;
    int imageSize = bytesPerRow * height;

    BMPFileHeader fileHeader{};
    fileHeader.bfType = 0x4D42;
    fileHeader.bfSize = 54 + imageSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = 54;

    BMPInfoHeader infoHeader{};
    infoHeader.biSize = 40;
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = imageSize;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать или открыть для сохранения файл " << filename << std::endl;
        return false;
    }

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    if (!file.good()) {
        std::cerr << "Ошибка: запиcm заголовков не удалась" << std::endl;
        return false;
    }

    std::vector<uint8_t> rowBuffer(bytesPerRow, 0);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcIndex = (y * width + x) * 3;
            rowBuffer[x * 3] = bgrData[srcIndex];
            rowBuffer[x * 3 + 1] = bgrData[srcIndex + 1];
            rowBuffer[x * 3 + 2] = bgrData[srcIndex + 2];
        }
        file.write(reinterpret_cast<const char*>(rowBuffer.data()), bytesPerRow);
        if (!file.good()) {
            std::cerr << "Ошибка: запись строки " << y << " не удалась" << std::endl;
            return false;
        }
    }

    return true;
}