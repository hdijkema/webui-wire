

all: release
	@echo done

release:
	cmake --build ./build/Release --target all

clean:
	cmake --build ./build/Release --target clean

flatpak: flatpak/webui-wire.flatpak
	@echo done


flatpak/webui-wire.flatpak:
	(cd flatpak;make)
