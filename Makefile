

all: release
	@echo done

release:
	cmake --build ./build/Release --target all

clean:
	cmake --build ./build/Release --target clean

flatpak:
	(cd flatpak;make)
