@echo off
echo Suppression des dossiers de builds...
rmdir /s /q dist build

echo Suppression du fichier spec...
del socket_server.spec

echo Construction avec PyInstaller...
pyinstaller --onefile socket_server.py

echo Le build est terminé.
