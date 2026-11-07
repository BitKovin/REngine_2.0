#pragma once

class QApplication;

namespace Theme
{
    // Dark palette plus one stylesheet; every colour used by custom widgets comes from here.
    void Apply(QApplication& app);
}
