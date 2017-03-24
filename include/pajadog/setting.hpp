#pragma once

#include <QString>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <memory>

namespace pajadog {

class ISettingData
{
public:
    ISettingData(const QString &_key)
        : key(_key)
    {
    }

    virtual int
    getInt()
    {
        return 0;
    }

    QString
    toString() const
    {
        QString ret;

        if (!this->key.isEmpty()) {
            ret += "Key: " + this->key + "\n";
        }
        if (!this->description.isEmpty()) {
            ret += "Description: " + this->description + "\n";
        }
        try {
            ret += QString("Value: %1\n").arg(this->getValue<int>());
        } catch (boost::bad_any_cast &) {
            std::cerr << "XD" << std::endl;
        }

        return ret.trimmed();
    }

    const QString &
    getKey() const
    {
        return this->key;
    }

    template <typename Type>
    Type
    getValue() const
    {
        boost::any ret = this->getValueImpl();

        return boost::any_cast<Type>(ret);
    }

protected:
    virtual boost::any getValueImpl() const = 0;

private:
    std::shared_ptr<ISettingData> parent = nullptr;

    const QString key;
    QString description;
};

template <typename Type>
class SettingData : public ISettingData
{
public:
    SettingData(const QString &_key, const Type &defaultValue)
        : ISettingData(_key)
        , data(defaultValue)
    {
        std::cout << "ctor SettingData: " << this->toString().toStdString()
                  << std::endl;
    }

    ~SettingData()
    {
        std::cout << "dtor SettingData: " << this->toString().toStdString()
                  << std::endl;
    }

    SettingData<Type> *
    setDescription(const QString &newDescription)
    {
        this->description = newDescription;

        return this;
    }

    Type data;

protected:
    virtual boost::any
    getValueImpl() const override
    {
        return boost::any(this->data);
    }
};

class Setting
{
public:
    Setting(ISettingData *_data);
    ~Setting();

private:
    std::shared_ptr<ISettingData> data;
};

}  // namespace pajadog
