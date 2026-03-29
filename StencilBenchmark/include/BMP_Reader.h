#ifndef BMP_H
#define BMP_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

#pragma pack(push, 1)

// Файловый заголовок
struct BMPFileHeader {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReceived1;
	uint16_t bfReceived2;
	uint32_t bfOffBits;
};

// Информационный заголовок
struct BMPInfoHeader {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

#pragma pack(pop)

// Функция загрузки изображения из файла
bool loadBMP(const std::string& filename, std::vector<uint8_t>& bgrData, 
	int& width, int& height);

// Функция сохранения изображения в файл
bool saveBMP(const std::string& filename, const std::vector<uint8_t>& bgrData, 
	int width, int height);

#endif