

all: release
	@echo done

release:
	cmake --build ./build/Release --target all

clean:
	cmake --build ./build/Release --target clean
	(cd flatpak;make clean)

flatpak: flatpak/webui-wire.flatpak
	@echo done


flatpak/webui-wire.flatpak:
	(cd flatpak;make)
