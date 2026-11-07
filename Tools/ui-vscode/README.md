# Engine UI Language (VS Code)

Auto complete, hover, diagnostics and colouring for the engine's UI files:

* **Markup** (`.xml` under `GameData/ui`) - language `uixml`
* **Styles** (`.css` under `GameData/ui`) - language `uicss`, the engine's CSS flavour

Everything it knows comes from the game's own `SaveData/UIStudio/uidb.json` (written when the game runs, the same file UI Studio uses), so new widgets, attributes, CSS properties and actions show up without changing the extension. Without a game folder it uses the bundled copy in `data/uidb.json`.

## Features

| | Markup | Styles |
|---|---|---|
| Colouring | tags, attributes, `${KEY}` localisation, actions in `onactivate`, `style=""` as CSS | selectors, states, properties, `var()`, `rgba()`, `@import`, `@keyframes` |
| Complete | tags (with required attributes), attributes, bool values, colours, `class` from your style sheets, action names, `${locale keys}`, file paths, closing tags, CSS inside `style=""` | properties, values from the property grammar, `var(--token)`, states, classes, ids, element types, easings, keyframe names, `@import` paths |
| Hover | tag and attribute docs, class origin, action docs | property grammar, initial value, inherited, token values |
| Problems | unknown tag / attribute (with "did you mean"), wrong bool / number / colour, missing required attribute, unclosed tags, missing files, unknown localisation key, unknown class | unknown property, undefined `var()`, unknown state or type, missing braces, bad `@import` |
| Go to definition | component / snippet tag, `src` file, class | class, `--variable`, `@keyframes`, `@import` |
| Other | outline, colour swatches (CSS and colour attributes) | outline, colour swatches |

Component files and snippets of the open document (`<snippet name=".." params="..">`, `<import src="..">`) become tags with their parameters.

## Build and install

```
cd tools/ui-vscode
npm install
npm run build
npm run install-local     # copies into ~/.vscode/extensions, then reload VS Code
```

To debug: open this folder in VS Code, press F5 (needs a launch configuration with `extensionDevelopmentPath`), or run `npm run watch` and reload.

Tests (start the real server and ask it questions, including the game's own files): `npm test`. `CORPUS=1 npm test` also lists everything the checker reports on all of the game's UI files, to spot false alarms.

## Setting

`engineUi.databasePath` - use a specific uidb.json instead of the one next to the game.

## Files

* `server/src/xml.ts`, `css.ts` - tolerant parsers that keep offsets (they must work while a file is half typed)
* `server/src/xmlFeatures.ts`, `cssFeatures.ts` - completion, hover, diagnostics, definitions, symbols, colours
* `server/src/project.ts` - finds the game folder, the database, the style sheet index
* `syntaxes/` - TextMate grammars for colouring
