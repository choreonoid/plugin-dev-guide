#include "BodyPositionItemView.h"
#include <cnoid/RootItem>
#include <cnoid/ItemList>
#include <cnoid/EigenUtil>
#include <cnoid/MenuManager>
#include <cnoid/Archive>

using namespace std;
using namespace cnoid;

BodyPositionItemView::BodyPositionItemView()
{
    setDefaultLayoutArea(BottomCenterArea);

    grid = new QGridLayout;
    setLayout(grid, 1.0);

    targetMode = All;
}

void BodyPositionItemView::onActivated()
{
    setTargetMode(targetMode);
}

void BodyPositionItemView::setTargetMode(TargetMode mode)
{
    if(mode != targetMode || !connectionForTargetDetection.connected()){
        targetMode = mode;
        if(isActive()){
            if(mode == All){
                connectionForTargetDetection =
                    BodyPositionItem::sigItemsInProjectChanged().connect(
                        [this](){ updateTargetItems(); });
            } else if(mode == Selected){
                connectionForTargetDetection =
                    RootItem::instance()->sigSelectedItemsChanged().connect(
                        [this](const ItemList<>&){ updateTargetItems(); });
            }
            updateTargetItems();
        }
    }
}

void BodyPositionItemView::onDeactivated()
{
    connectionForTargetDetection.disconnect();
}

void BodyPositionItemView::updateTargetItems()
{
    ItemList<BodyPositionItem> items;
    if(targetMode == All){
        items = RootItem::instance()->descendantItems<BodyPositionItem>();
    } else if(targetMode == Selected){
        items = RootItem::instance()->selectedItems<BodyPositionItem>();
    }

    size_t prevSize = interfaceUnits.size();
    interfaceUnits.resize(items.size());

    for(size_t i = prevSize; i < interfaceUnits.size(); ++i){
        auto& unit = interfaceUnits[i];
        unit.reset(new InterfaceUnit);

        unit->nameLabel = new QLabel(this);
        
        unit->heightSlider = new Slider(Qt::Horizontal, this);
        unit->heightSlider->setRange(1, 3000);
        unit->connections.add(
            unit->heightSlider->sigValueChanged().connect(
                [=](int value){ onHeightSliderValueChanged(i, value); }));

        unit->orientationDial = new Dial(this);
        unit->orientationDial->setRange(-180, 180);
        unit->connections.add(
            unit->orientationDial->sigValueChanged().connect(
                [=](int value){ onOrientationDialValueChanged(i, value); }));

        unit->storeButton = new PushButton("Store", this);
        unit->storeButton->sigClicked().connect(
            [=](){ onStoreButtonClicked(i); });
        
        unit->restoreButton = new PushButton("Restore", this);
        unit->restoreButton->sigClicked().connect(
            [=](){ onRestoreButtonClicked(i); });

        grid->addWidget(unit->nameLabel, i, 0);
        grid->addWidget(unit->heightSlider, i, 1);
        grid->addWidget(unit->orientationDial, i, 2);
        grid->addWidget(unit->storeButton, i, 3);
        grid->addWidget(unit->restoreButton, i, 4);
    }

    itemConnections.disconnect();
    for(size_t i =0; i < items.size(); ++i){
        auto& item = items[i];
        auto& unit = interfaceUnits[i];
        unit->item = item;
        unit->nameLabel->setText(item->name().c_str());
        itemConnections.add(
            item->sigUpdated().connect(
                [=](){ updateInterface(i); }));
        updateInterface(i);
    }
}

void BodyPositionItemView::updateInterface(int index)
{
    auto& unit = interfaceUnits[index];
    auto& item = unit->item;
    unit->connections.block();
    unit->heightSlider->setValue(item->flagHeight() * 1000);
    auto rpy = rpyFromRot(item->position().linear());
    unit->orientationDial->setValue(degree(rpy.z()));
    unit->connections.unblock();
}

void BodyPositionItemView::onHeightSliderValueChanged(int index, int value)
{
    interfaceUnits[index]->item->setFlagHeight(value / 1000.0);
}

void BodyPositionItemView::onOrientationDialValueChanged(int index, int value)
{
    auto item = interfaceUnits[index]->item;
    auto T = item->position();
    auto rpy = rpyFromRot(T.linear());
    rpy.z() = radian(value);
    T.linear() = rotFromRpy(rpy);
    item->setPosition(T);
}

void BodyPositionItemView::onStoreButtonClicked(int index)
{
    interfaceUnits[index]->item->storeBodyPosition();
}

void BodyPositionItemView::onRestoreButtonClicked(int index)
{
    interfaceUnits[index]->item->restoreBodyPosition();
}

void BodyPositionItemView::onAttachedMenuRequest(cnoid::MenuManager& menuManager)
{
    auto modeCheck = menuManager.addCheckItem("Selected body position items only");
    modeCheck->setChecked(targetMode == Selected);
    modeCheck->sigToggled().connect(
        [this](bool on){ setTargetMode(on ? Selected : All); });
    menuManager.addSeparator();
}

bool BodyPositionItemView::storeState(cnoid::Archive& archive)
{
    archive.write("target_mode", (targetMode == All) ? "all" : "selected");
    return true;
}

bool BodyPositionItemView::restoreState(const cnoid::Archive& archive)
{
    string mode;
    if(archive.read("target_mode", mode)){
        if(mode == "all"){
            setTargetMode(All);
        } else if(mode == "selected"){
            setTargetMode(Selected);
        }
    }
    return true;
}

BodyPositionItemView::InterfaceUnit::~InterfaceUnit()
{
    delete nameLabel;
    delete heightSlider;
    delete orientationDial;
    delete storeButton;
    delete restoreButton;
}
