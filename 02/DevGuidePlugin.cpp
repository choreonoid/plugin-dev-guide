#include <cnoid/Plugin>
#include <cnoid/MessageView>
#include <cnoid/TimeBar>
#include <fmt/format.h>

using namespace cnoid;

class DevGuidePlugin : public Plugin
{
    ScopedConnection connection;
    
public:
    DevGuidePlugin() : Plugin("DevGuide")
    {

    }
    
    virtual bool initialize() override
    {
        connection =
            TimeBar::instance()->sigTimeChanged().connect(
                [this](double time){ return onTimeChanged(time); });
        return true;
    }

    bool onTimeChanged(double time)
    {
        MessageView::instance()->putln(fmt::format("Current time is {}", time));
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
