#include <musicplaylist/Musicdb.h>
#include <stdexcept>

mspl::Database::Database(const char* metadataPath) {
	m_metadataFile.open(metadataPath, std::ios::binary | std::ios::in | std::ios::out);
	if (!m_metadataFile.is_open()) {
		std::ofstream temp(metadataPath, std::ios::binary);
		temp.close();
		m_metadataFile.open(metadataPath, std::ios::binary | std::ios::in | std::ios::out);
		if (!m_metadataFile.is_open()) {
			throw std::runtime_error("Failed to read database file");
		}
	}

	m_metadataFile.seekg(0, std::ios::end);
	size_t size = m_metadataFile.tellg();
	m_metadataFile.seekg(0);

	if (size == 0) {
		return;
	}
	std::string buffer;
	buffer.resize(size);
	m_metadataFile.read(buffer.data(), size);

	if (!m_metadataFile) {
		m_metadataFile.close();
		throw std::runtime_error("Failed to read database file");
	}
	const char* readData = buffer.data();

	uint32_t elementCount = *(const uint32_t*)readData;
	readData += sizeof(uint32_t);

	for (uint32_t i = 0; i < elementCount; ++i) {
		DatabaseElement element{};
		element.durationSeconds = *(const double*)readData;
		readData += sizeof(double);

		uint32_t nameLength = *(const uint32_t*)readData;
		readData += sizeof(uint32_t);
		element.name.reserve(nameLength);
		element.name.assign(readData, nameLength);
		readData += nameLength;

		uint32_t pathLength = *(const uint32_t*)readData;
		readData += sizeof(uint32_t);
		std::string pathString;
		pathString.reserve(pathLength);
		pathString.assign(readData, pathLength);
		element.audioPath = pathString;
		readData += pathLength;

		uint32_t artistCount = *(const uint32_t*)readData;
		element.artists.reserve(artistCount);
		readData += sizeof(uint32_t);

		for (uint32_t j = 0; j < artistCount; ++j) {
			uint32_t artistNameLength = *(const uint32_t*)readData;
			readData += sizeof(uint32_t);
			std::string artist;
			artist.reserve(artistNameLength);
			artist.assign(readData, artistNameLength);
			element.artists.emplace_back(std::move(artist));
			readData += artistNameLength;
		}
		m_metadataSet.insert(std::move(element));
	}
}

mspl::Database::~Database() {
	if (m_metadataFile.is_open()) {
		m_metadataFile.close();
	}
}

void mspl::Database::Add(const DatabaseElement& element) {
	constexpr int SIZEOF_UINT32 = sizeof(uint32_t);

	auto it = m_metadataSet.find(element);
	if (it != m_metadataSet.cend()) {
		return;
	}
	m_metadataFile.seekp(0, std::ios::beg);
	uint32_t size = (uint32_t)m_metadataSet.size() + 1;
	m_metadataFile.write((const char*)&size, SIZEOF_UINT32);

	m_metadataFile.seekp(0, std::ios::end);
	m_metadataFile.write((const char*)&element.durationSeconds, sizeof(double));
	{
		uint32_t nameLength = (uint32_t)element.name.length();
		m_metadataFile.write((const char*)&nameLength, SIZEOF_UINT32);
		m_metadataFile.write(element.name.c_str(), nameLength);
	}
	{
		std::string pathStr = element.audioPath.string();
		uint32_t pathLength = (uint32_t)pathStr.length();
		m_metadataFile.write((const char*)&pathLength, SIZEOF_UINT32);
		m_metadataFile.write(pathStr.c_str(), pathLength);
	}
	uint32_t artistCount = (uint32_t)element.artists.size();
	m_metadataFile.write((const char*)&artistCount, SIZEOF_UINT32);
	for (uint32_t i = 0; i < artistCount; ++i) {
		uint32_t artistNameLength = (uint32_t)element.artists[i].length();
		m_metadataFile.write((const char*)&artistNameLength, SIZEOF_UINT32);
		m_metadataFile.write(element.artists[i].c_str(), artistNameLength);
	}
	m_metadataSet.insert(element);
	if (!m_metadataFile) {
		throw std::runtime_error("Failed to write database file");
	}
}

const mspl::DatabaseElement* mspl::Database::Get(const std::string& name) const {
	DatabaseElement elem;
	elem.name = name;
	auto it = m_metadataSet.find(elem);

	if (it != m_metadataSet.cend()) {
		return &(*it);
	}
	return nullptr;
}