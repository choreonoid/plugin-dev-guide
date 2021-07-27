#ifndef DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_VIEW_H
#define DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_VIEW_H

#include "BodyPositionItem.h"
#include <cnoid/View>
#include <cnoid/ConnectionSet>
#include <cnoid/Slider>
#include <cnoid/Dial>
#include <cnoid/Buttons>
#include <QLabel>
#include <QGridLayout>
#include <vector>
#include <memory>

class BodyPositionItemView : public cnoid::View
{
public:
    BodyPositionItemView();

protected:
    virtual void onActivated() override;
    virtual void onDeactivated() override;
    virtual void onAttachedMenuRequest(cnoid::MenuManager& menuManager) override;
    virtual bool storeState(cnoid::Archive& archive) override;
    virtual bool restoreState(const cnoid::Archive& archive) override;

private:
    enum TargetMode { All, Selected };
    void setTargetMode(TargetMode mode);
    void updateTargetItems();
    void updateInterface(int index);
    void onHeightSliderValueChanged(int index, int value);
    void onOrientationDialValueChanged(int index, int value);
    void onStoreButtonClicked(int index);
    void onRestoreButtonClicked(int index);
    
    TargetMode targetMode;
    cnoid::Connection connectionForTargetDetection;
    cnoid::ScopedConnectionSet itemConnections;

    struct InterfaceUnit
    {
        BodyPositionItemPtr item;
        QLabel* nameLabel;
        cnoid::Slider* heightSlider;
        cnoid::Dial* orientationDial;
        cnoid::PushButton* storeButton;
        cnoid::PushButton* restoreButton;
        cnoid::ConnectionSet connections;

        ~InterfaceUnit();
    };

    std::vector<std::unique_ptr<InterfaceUnit>> interfaceUnits;
    
    QGridLayout* grid;
};

#endif // DEVGUIDE_PLUGIN_BODY_POSITION_ITEM_VIEW_H
