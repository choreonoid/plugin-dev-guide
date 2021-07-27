#include <cnoid/Plugin>
#include <cnoid/MessageView>

using namespace cnoid;

class DevGuidePlugin : public Plugin
{
public:
    DevGuidePlugin() : Plugin("DevGuide")
    {

    }
    
    virtual bool initialize() override
    {
        MessageView::instance()->putln("Hello World!");
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
