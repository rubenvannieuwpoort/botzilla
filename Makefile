.PHONY: match situation test tournament wrapper upload clean

situation:
	g++ src/situation.cpp -O3 -o situation

tournament:
	g++ src/tournament.cpp -O3 -o tournament

wrapper:
	g++ src/wrapper.cpp -O3 -o wrapper -static
	cp wrapper bot.bin
	zip bot.zip bot.bin

upload: wrapper
	curl -X POST "https://infinibattle.infi.nl/Api/UploadBot/94e58c4f-96c7-467c-904f-a2c9bf9043fa" -F 'File=@bot.zip'
	rm bot.zip bot.bin

clean:
	rm -rf situation tournament wrapper bot.bin bot.zip

