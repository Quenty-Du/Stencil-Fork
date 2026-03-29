#include <BMP_Reader.h>


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
		std::cerr << "Ошибка: ридер поддерживает только стандартный формат BMP, 
		размер заголовка которого 40 байт" << std::endl;
		return false;
	}

	if (infoHeader.biBitCount != 24) {
		std::cerr << "Ошибка: поддерживаются только 24-битные BMP 
		(3 слоя: R, G, B, по 8 бит на каждый)" << endl;
		return false;
	}

	if (infoHeader.biCompression != 0) {
		std::cerr << "Ошибка: ридер не поддерживает сжатые файлы" << endl;
		return false;
	}

	width = infoHeader.biWidth;

	height = (infoHeader.biHeight > 0) ? infoHeader.biHeight :
	-infoHeader.biHeight;

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


bool saveBMP(const std::string& filename, const std::vector<uint8_t>& bgrData, 
	int width, int height) {
	int bytesPerRow = (width * 3 + 3) & ~3;
	int imageSize = bytesOerRow * height;

	BMPFileHeader fileHeader{};
	fileHeader.bfType = 0x4D42;
	fileHeader.bfSize = 54 + imageSize;
	fileHeader.bfReceived1 = 0;
	fileHeader.bfReceived2 = 0;
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

	std::ofstream file(filename, std::ios:binary);
	if (!file.is_open()) {
		std::cerr << "Ошибка: не удалось создать или открыть для сохранения файл "
		<< filename << std::endl;
		return false;
	}

	file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
	file.write(reinterpret_cast<const char*>(&infoheader), sizeof(infoHeader));

	if (!file.good()) {
		std:cerr << "Ошибка: запиcm заголовков не удалась" << std::endl;
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