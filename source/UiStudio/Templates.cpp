#include "Templates.h"

const QList<FileTemplate>& Templates::All()
{
    static const QList<FileTemplate> list = {
        { "Empty panel", "A full-screen panel with a title and its own style sheet. A good place to start.", ".xml",
R"(<root>
  <styles>
    <include src="%2/%1.css"/>
  </styles>
  <Panel id="%1" class="%1-root">
    <Label class="%1-title" text="%1"/>
  </Panel>
</root>
)" },
        { "Menu page", "A column of buttons, like a pause or main menu.", ".xml",
R"(<root>
  <styles>
    <include src="%2/%1.css"/>
  </styles>
  <Panel id="%1" class="%1-root">
    <Panel class="%1-column">
      <Label class="%1-title" text="%1"/>
      <Button id="first" class="%1-button" default-focus="true">
        <Label text="Play"/>
      </Button>
      <Button class="%1-button">
        <Label text="Options"/>
      </Button>
      <Button class="%1-button">
        <Label text="Quit"/>
      </Button>
    </Panel>
  </Panel>
</root>
)" },
        { "Style sheet", "A style sheet with a few starter rules to adjust.", ".css",
R"(.%1-root { width: 100%; height: 100%; }
.%1-column { flow-children: down; gap: 12px; horizontal-align: center; vertical-align: center; }
.%1-title { font-size: 40px; color: #ffffff; }
.%1-button { padding: 8px 20px; background-color: #2a2d36; border-radius: 6px; }
.%1-button:hover, .%1-button:focus { background-color: #4a8af4; }
)" },
    };
    return list;
}

QString Templates::Render(const FileTemplate& t, const QString& baseName, const QString& folder)
{
    QString out = t.content;
    out.replace("%1", baseName);
    out.replace("%2", folder);
    return out;
}
