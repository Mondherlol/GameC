set render=src\engine\render\render.c src\engine\render\render_init.c src\engine\render\render_util.c src\engine\render\render_text.c
set io=src\engine\io\io.c
set config=src\engine\config\config.c
set input=src\engine\input\input.c
set time=src\engine\time\time.c
set physics=src\engine\physics\physics.c
set array_list=src\engine\array_list\array_list.c
set entity=src\engine\entity\entity.c
set animation=src\engine\animation\animation.c
set my_curl=src\engine\my_curl\my_curl.c
set scenes=src\engine\scenes\scenes.c src\engine\scenes\menu_screen.c src\engine\scenes\score_screen.c src\engine\scenes\game_over_screen.c src\engine\scenes\username_menu_screen.c
set audio=src\engine\audio\audio.c
set scores= src\engine\scores\scores.c
set files=src\glad.c src\main.c src\engine\global.c %render% %io% %config% %input% %time% %physics% %array_list% %entity% %animation% %my_curl% %scenes% %audio% %scores%
set libs=../lib/SDL2main.lib ../lib/SDL2.lib ../lib/libcurl.a ../lib/libcurl.dll.a ../lib/freetype.lib ../lib/SDL2_mixer.lib

CL /Zi /I ../include %files% /link %libs% /OUT:mygame.exe