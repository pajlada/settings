#pragma once

#include <pajlada/settings/setting.hpp>

#include <QSpinBox>

namespace pajlada {
namespace Settings {

template <typename WidgetType, typename SettingType>
inline void
connectToWidget(WidgetType *widget, Setting<SettingType> &setting,
                bool setInitialValue = true)
{
}

template <typename SettingType>
inline void
connectToWidget(QSpinBox *widget, Setting<SettingType> &setting,
                bool setInitialValue = true)
{
    if (setInitialValue) {
        widget->setValue(setting.getValue());
    }

    // Connect widget onChange to setting
    QObject::connect(
        widget, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
        [&setting](int newValue) {
            setting = newValue;  //
        });

    // Connect setting onChange to widget
    setting.getValueChangedSignal().connect([widget](const auto &newValue) {
        widget->setValue(newValue);  //
    });
}

template <typename SettingType>
inline void
connectToWidget(QComboBox *widget, Setting<SettingType> &setting,
                bool setInitialValue = true)
{
    if (setInitialValue) {
        widget->setCurrentText(QString::fromStdString(setting.getValue()));
    }

    // Connect widget onChange to setting
    QObject::connect(widget,
                     static_cast<void (QComboBox::*)(const QString &)>(
                         &QComboBox::currentIndexChanged),
                     [&setting](const QString &newValue) {
                         setting = newValue.toStdString();  //
                     });

    // Connect setting onChange to widget
    setting.getValueChangedSignal().connect(
        [widget](const std::string &newValue) {
            widget->setCurrentText(QString::fromStdString(newValue));  //
        });
}

}  // namespace Settings
}  // namespace pajlada
