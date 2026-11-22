@echo off
rem Exports everything the game needs of the dynamite and the molotov into GameData: the viewmodels, the thrown models and the
rem third-person models. Edit BLENDER if Blender lives somewhere else.
set BLENDER=D:\SteamLibrary\steamapps\common\Blender\blender.exe
set HERE=%~dp0
"%BLENDER%" -b "%HERE%dynamite.blend" --python "%HERE%export_throwable.py" || exit /b 1
"%BLENDER%" -b "%HERE%molotov.blend" --python "%HERE%export_throwable.py" || exit /b 1
"%BLENDER%" -b "%HERE%thrown\dynamite.blend" --python "%HERE%export_static.py" || exit /b 1
"%BLENDER%" -b "%HERE%thrown\molotov.blend" --python "%HERE%export_static.py" || exit /b 1
"%BLENDER%" -b "%HERE%thirdPerson\dynamite.blend" --python "%HERE%export_static.py" || exit /b 1
"%BLENDER%" -b "%HERE%thirdPerson\molotov.blend" --python "%HERE%export_static.py" || exit /b 1
echo Done.
