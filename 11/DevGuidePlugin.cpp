#include "BodyPositionItem.h"
#include "BodyPositionItemView.h"
#include <cnoid/Plugin>
#include <cnoid/ViewManager>
#include <cnoid/ToolBar>
#include <cnoid/RootItem>
#include <cnoid/ItemList>

using namespace cnoid;

class DevGuidePlugin : public Plugin
{
public:
    DevGuidePlugin()
        : Plugin("DevGuide")
    {
        require("Body");
    }
        
    virtual bool initialize() override
    {
        BodyPositionItem::initializeClass(this);

        viewManager().registerClass<BodyPositionItemView>(
            "BodyPositionItemView", "Body Position Items");
        
        auto toolBar = new ToolBar("BodyPositionBar");
        toolBar->addButton("Store Body Positions")->sigClicked().connect(
            [this](){ storeBodyPositions(); });
        toolBar->addButton("Restore Body Positions")->sigClicked().connect(
            [this](){ restoreBodyPositions(); });
        toolBar->setVisibleByDefault();
        addToolBar(toolBar);

        return true;
    }
            
    void storeBodyPositions()
    {
        for(auto& item : RootItem::instance()->selectedItems<BodyPositionItem>()){
            item->storeBodyPosition();
        }
    }
    
    void restoreBodyPositions()
    {
        for(auto& item : RootItem::instance()->selectedItems<BodyPositionItem>()){
            item->restoreBodyPosition();
        }
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(DevGuidePlugin)
