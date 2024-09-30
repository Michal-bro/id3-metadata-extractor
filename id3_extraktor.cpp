#include <iostream>
#include <string>
#include <fstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
#include <curl/curl.h>
#include <filesystem>

// Funkce pro stahování souboru z internetu pomocí libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* ofs = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    ofs->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

void downloadFile(const std::string& url, const std::string& outputPath) {
    CURL* curl;
    CURLcode res;
    std::ofstream ofs(outputPath, std::ios::binary);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "Stahování souboru selhalo: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

// Funkce pro extrakci ID3 tagů (TIT2 a APIC)
void extractID3Tags(const std::string& mp3Path) {
    TagLib::MPEG::File mp3File(mp3Path.c_str());
    TagLib::ID3v2::Tag* tag = mp3File.ID3v2Tag();

    if (!tag) {
        std::cerr << "MP3 soubor neobsahuje ID3v2 tagy." << std::endl;
        return;
    }

    // Získání TIT2 (název skladby)
    TagLib::ID3v2::Frame* titleFrame = tag->frameListMap()["TIT2"].front();
    std::string title = titleFrame ? titleFrame->toString().to8Bit(true) : "Unknown Title";

    // Vytvoření adresáře na základě TIT2
    std::filesystem::create_directory(title);

    // Získání APIC (obrázek)
    TagLib::ID3v2::FrameList frameList = tag->frameListMap()["APIC"];
    if (!frameList.isEmpty()) {
        TagLib::ID3v2::AttachedPictureFrame* pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
        std::string imagePath = title + "/" + title + ".jpg";
        std::ofstream imageFile(imagePath, std::ios::binary);
        imageFile.write(pictureFrame->picture().data(), pictureFrame->picture().size());
        imageFile.close();
        std::cout << "Obrázek extrahován a uložen do: " << imagePath << std::endl;
    } else {
        std::cerr << "Obrázek (APIC) nebyl nalezen v MP3 souboru." << std::endl;
    }

    // Přesunutí MP3 souboru do nového adresáře
    std::filesystem::rename(mp3Path, title + "/" + mp3Path.substr(mp3Path.find_last_of("/\\") + 1));
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Použití: " << argv[0] << " <mp3 soubor nebo URL>" << std::endl;
        return 1;
    }

    std::string input = argv[1];
    std::string mp3FilePath;

    // Pokud je vstup URL, stáhneme MP3 soubor
    if (input.rfind("http", 0) == 0) {
        mp3FilePath = "downloaded.mp3";
        downloadFile(input, mp3FilePath);
    } else {
        mp3FilePath = input;
    }

    // Extrakce ID3 tagů a obrázku
    extractID3Tags(mp3FilePath);

    return 0;
}
