#include <cnoid/Plugin>
#include <cnoid/ItemManager>
#include <cnoid/Item>
#include <cnoid/RootItem>
#include <cnoid/BodyItem>
#include <cnoid/ItemList>
#include <cnoid/ToolBar>
#include <fmt/format.h>

using namespace std;
using namespace fmt;
using namespace cnoid;

class BodyPositionItem : public Item
{
    BodyItem* bodyItem;
    Isometry3 position;
    
public:
    BodyPositionItem()
    {
        bodyItem = nullptr;
        position.setIdentity();
    }
    
    BodyPositionItem(const BodyPositionItem& org)
        : Item(org)
    {
        bodyItem = nullptr;
        position = org.position;
    }
    
    virtual Item* doDuplicate() const override
    {
        return new BodyPositionItem(*this);
    }

    virtual void onTreePathChanged() override
    {
        auto newBodyItem = findOwnerItem<BodyItem>();
        if(newBodyItem && newBodyItem != bodyItem){
            bodyItem = newBodyItem;
            mvout()
                << format("BodyPositionItem \"{0}\" has been attached to {1}.",
                          name(), bodyItem->name())
                << endl;
        }
    }

    void storeBodyPosition()
    {
        if(bodyItem){
            position = bodyItem->body()->rootLink()->position();
            mvout()
                << format("The current position of {0} has been stored to {1}.",
                          bodyItem->name(), name())
                << endl;
        }
    }
            
    void restoreBodyPosition()
    {
        if(bodyItem){
            bodyItem->body()->rootLink()->position() = position;
            bodyItem->notifyKinematicStateChange(true);
            mvout()
                << format("The position of {0} has been restored from {1}.",
                          bodyItem->name(), name())
                << endl;
        }
    }
};

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
        itemManager()
            .registerClass<BodyPositionItem>("BodyPositionItem")
            .addCreationPanel<BodyPositionItem>();
        
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
