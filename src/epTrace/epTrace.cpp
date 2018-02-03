#define _CRT_SECURE_NO_WARNINGS

#include "../BlackBone/Config.h"
#include "../BlackBone/Process/Process.h"
#include "../BlackBone/PE/PEImage.h"
#include "../BlackBone/Misc/Utils.h"
#include "../BlackBone/Misc/DynImport.h"
#include "../BlackBone/Patterns/PatternSearch.h"
#include <cstdio>
#include <string>
#include <cstdint>
#include <regex>
#include <fstream>
#include <codecvt>
#include "ewin.h"

using namespace blackbone;

int lineNo = 0;

void Print(const char *format, ...) {
	char str[1024];

	va_list args;
	va_start(args, format);
	vsprintf_s(str, 1024, format, args);
	EWtext(0, lineNo * 16, str);
	lineNo++;
}


// https://stackoverflow.com/questions/7153935/how-to-convert-utf-8-stdstring-to-utf-16-stdwstring
std::wstring utf8_to_utf16(const std::string& utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F)
		{
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)
		{
			throw std::logic_error("not a UTF-8 string");
		}
		else if (ch <= 0xDF)
		{
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF)
		{
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7)
		{
			uni = ch & 0x07;
			todo = 3;
		}
		else
		{
			throw std::logic_error("not a UTF-8 string");
		}
		for (size_t j = 0; j < todo; ++j)
		{
			if (i == utf8.size())
				throw std::logic_error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				throw std::logic_error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			throw std::logic_error("not a UTF-8 string");
		if (uni > 0x10FFFF)
			throw std::logic_error("not a UTF-8 string");
		unicode.push_back(uni);
	}
	std::wstring utf16;
	for (size_t i = 0; i < unicode.size(); ++i)
	{
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF)
		{
			utf16 += (wchar_t)uni;
		}
		else
		{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}

bool hexParse(const char* header, std::vector<uint8_t>* patternV) {
	for (int i = 0; i < 16; i++) {
		const char* hexS = "0123456789abcdef";
		auto x1 = strchr(hexS, header[i * 2 + 0]);
		auto x2 = strchr(hexS, header[i * 2 + 1]);
		if (x1 == 0 || x2 == 0) {
			printf("Invalid header\n");
			return false;
		}
		auto d1 = x1 - hexS, d2 = x2 - hexS;
		patternV->push_back(d1 << 4 | d2);
	}
	return true;
}

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		printf("Usage: epTrace [source.epmap]\n");
		return 1;
	}
	std::string ifname = argv[1];
	FILE *fp = fopen(ifname.c_str(), "r");
	if (fp == NULL) {
		printf("Cannot open epmap file!\n");
		return 2;
	}

	// Read epmap header
	char header1[33] = { 0 }, header2[33] = { 0 };
	fscanf(fp, "H0: %32s\n", header1);
	fscanf(fp, "H1: %32s\n", header2);
	if (strlen(header1) != 32 || strlen(header2) != 32) {
		printf("invald epmap file header.\n");
		fclose(fp);
		return 3;
	}

	// Read epmap data
	std::unordered_map<int, std::string> mapData;
	uint32_t val;
	char path[261];

	while (fscanf(fp, " - %08X : %260s\n", &val, path) == 2) {
		mapData[val] = path;
	}

	fclose(fp);

	// Hex parse header
	std::vector<uint8_t> patternV;
	hexParse(header2, &patternV);

	// Find pattern
	Process processSC;
	if ((NTSTATUS)processSC.Attach(L"StarCraft.exe") < 0) {
		printf("Cannot find StarCraft.exe process!\n");
		return 5;
	}

	PatternSearch ps1(patternV);
	printf("Searching trace table header..\n");
	std::vector<ptr_t> results;
	ps1.SearchRemoteWhole(processSC, false, 0, results);
	// Try searching...
	int retryn = 0;
	while (results.size() == 0) {
		Sleep(1000);
		if (!processSC.valid()) {
			printf("Process unexpectedly quit\n");
			return 5;
		}
		printf(" - Retrying... [%d]\r", ++retryn);
		ps1.SearchRemoteWhole(processSC, false, 0, results);
	}

	if (results.size() >= 2) {
		printf("Too many trace tables.\n");
		return 7;
	}
	auto traceTableStart = results[0];

	auto succeededSampleNum = 0;
	auto failedSampleNum = 0;

	const int maxLines = 24;

	auto& memorySC = processSC.memory();
	std::set<uint32_t> sampledCheckpointSet;
	std::map<uint32_t, int> sampleHitCount;
	uint32_t stackTrace[2048] = { 0 };
	uint32_t logTraceTime = GetTickCount() + 20;

	EWinit(GetModuleHandle(nullptr), SW_SHOW, 800, 600, "Log window", DefWindowProc, 0);

	while (EWcheckmsg()) {
		lineNo = 0;
		// Check header
		char currentHeaderValue[16];
		if (!processSC.valid()) break;  // SC closed
		if (memorySC.Read(traceTableStart, 16, currentHeaderValue) != STATUS_SUCCESS) break;  // Header not available : Maybe SC quitted game
		else if (memcmp(currentHeaderValue, patternV.data(), 16) != 0) break;  // Header changed. Game quitted and the memory is used for other purpose.

		if (memorySC.Read(traceTableStart + 20, 2048 * 4, stackTrace) == 0) {
			sampledCheckpointSet.clear();
			succeededSampleNum++;

			auto currentTime = GetTickCount();
			if (currentTime >= logTraceTime) {
				EWclear();
			}
			Print("Sample got %d / Failed %d", succeededSampleNum, failedSampleNum);

			int stackDepth = 0;
			while (stackTrace[stackDepth] != 0) {
				sampledCheckpointSet.insert(stackTrace[stackDepth]);
				stackDepth++;
			}
			for (uint32_t checkPoint : sampledCheckpointSet) {
				auto it = sampleHitCount.find(checkPoint);
				if (it != sampleHitCount.end()) it->second++;
				else sampleHitCount[checkPoint] = 1;
			}

			if (currentTime >= logTraceTime) {
				logTraceTime = currentTime + 20;
				int i;
				for (i = max(0, stackDepth - maxLines); i < stackDepth; i++) {
					Print(" - [%2d] %s", i, mapData[stackTrace[i]].c_str());
				}

				for (; i < maxLines; i++) {
					Print("");
				}
			}
		}
		else {
			Print("Sample got %d / Failed %d", succeededSampleNum, failedSampleNum);

			failedSampleNum++;
		}

		if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_ESCAPE)) break;
		EWshow();
		Sleep(1);
	}

	EWfin();
	EWcheckmsg();


	std::regex epScriptLogRegex("(.+)\\|(.+)\\|(\\d+)");
	std::smatch matches;

	using lnmap = std::map<int, int>;
	std::unordered_map<std::string, lnmap> fnmap;
	for (const auto& it : sampleHitCount) {
		if (std::regex_match(mapData[it.first], matches, epScriptLogRegex)) {
			auto filename = matches[1].str();
			auto funcname = matches[2].str();
			auto lineno = std::stoi(matches[3].str());

			fnmap[filename][lineno] = it.second;
		}
	}


	// Write profiling data

	auto ofname = ifname + ".prof";
	std::ofstream os(ofname.c_str());
	if (!os) {
		printf(" - Cannot output profiling result to \"%s\".\n", ofname.c_str());
		return 8;
	}

	for (const auto& it : fnmap) {
		const auto& iSrcName = it.first;

		try {
			std::wstring u16SrcName = utf8_to_utf16(iSrcName);

			std::ifstream is(u16SrcName.c_str());
			if (!is) {
				printf(" - [I] Cannot open input source \"%s\"!\n", iSrcName.c_str());
				continue;
			}

			printf("Writing profiling info about \"%S\"\n", u16SrcName.c_str());

			int lno = 0;
			std::string lstr;
			const auto& lnmap = it.second;

			os << "[File \"" << iSrcName << "\"]\n";
			os << "-------------------------------------------------\n\n";

			while (std::getline(is, lstr)) {
				lno++;

				auto it2 = lnmap.find(lno);
				if (it2 != lnmap.end()) {
					char t[15];
					sprintf(t, "[%7d ]    ", it2->second);
					os.write(t, 14);
				}
				else {
					os.write("              ", 14);
				}
				os << lstr << "\n";
			}

			os << "\n\n\n\n";
		}
		catch (std::logic_error e) {
			printf("Not a valid utf-8 filename: %s\n", iSrcName.c_str());
		}
	}

	os.close();
	return 0;
}
