# ![PDF Mix Tool logo](https://scarpetta.eu/pdfmixtool/icon.svg) PDF Mix Tool

PDF Mix Tool is a simple and lightweight application that allows you to
perform common editing operations on PDF files.

![Screenshot](https://scarpetta.eu/pdfmixtool/merge_files.png)

Base operations it can perform are the following:

- Merge two or more files specifying a page set for each of them
- Rotate pages
- Composite more pages onto a single one (N-up)
- Combinations of all of the above

Besides, it can also mix files alternating their pages, generate booklets,
add white pages to a PDF file, delete pages from a PDF file, extract pages
from a PDF file, edit the PDF document information, extract images from a PDF file.

## Useful links
[Changelog/Roadmap](CHANGELOG.md)

[Website](https://scarpetta.eu/pdfmixtool/): here you can find usage
guides, building instructions and all other information.

Help translating on Weblate: <a href="https://hosted.weblate.org/engage/pdf-mix-tool/?utm_source=widget">
    <img src="https://hosted.weblate.org/widgets/pdf-mix-tool/-/svg-badge.svg" alt="Translations status" />
</a>

## Installation
### Packaging status
Here you can check if PDF Mix Tool is available in your distribution:

[![Packaging status](https://repology.org/badge/vertical-allrepos/pdfmixtool.svg)](https://repology.org/project/pdfmixtool/versions)

## Building
To build PDF Mix Tool you need the following libraries and tools:

- **Qt** (base, svg and tools) >= **5.11**
- **qpdf** (version >= **10.0.0** recommended)
- **ImageMagick (Magick++)**
- **CMake** >= **3.6**

Run the following commands inside the git repository directory:

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make
```

Starting from version 1.0, Qt 6 is supported. To build PDF Mix Tool using
Qt 6, run the following commands inside the git repository directory:

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DQT_VERSION=6
make
```

Run the following command to install PDF Mix Tool system-wide:

```
sudo make install
```
