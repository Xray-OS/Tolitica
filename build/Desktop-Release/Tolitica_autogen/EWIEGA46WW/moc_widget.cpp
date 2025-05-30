/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../widget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6WidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto Widget::qt_create_metaobjectdata<qt_meta_tag_ZN6WidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Widget",
        "autostart",
        "",
        "cleanOrphans",
        "cleanPkgCache",
        "systemUpdate",
        "removeDBLock",
        "tweaksSetupConnections",
        "QStackedWidget*",
        "stackedWidget",
        "QPushButton*",
        "tweaksButton",
        "backButton",
        "cleanOrphansButton",
        "cleanPkgCacheButton",
        "updateSystemButton",
        "removeDBLockButton",
        "QCheckBox*",
        "bluetoothToggle",
        "appArmorToggle",
        "rankMirrorsButton",
        "adaGamingMeta",
        "removeAdaGamingMeta",
        "adaDevelopmentMeta",
        "removeAdaDevelopmentMeta",
        "chaoticAUR",
        "removeChaoticAUR",
        "runCommand",
        "cmd",
        "checkChaoticAURStatus",
        "backupPacmanConfig",
        "addVMware",
        "vmwButton",
        "vmwareStatus",
        "vmwareServiceStatus",
        "addonsSetupConnections",
        "addonsButton",
        "addonsBackButton",
        "adaGamingMetaButton",
        "adaDevelopmentButton",
        "chaoticAURButton",
        "flatpakToggle",
        "snapdToggle",
        "terminalSetupConnections",
        "terminalButton",
        "terminalBackButton",
        "terminalThemeButton",
        "changeShellButton",
        "QComboBox*",
        "shellComboBox",
        "QLabel*",
        "shellLabel",
        "disableTermTheme",
        "checkTermThemingStatus",
        "mountDrivesSetupConnections",
        "QToolButton*",
        "mountDriveButton"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'autostart'
        QtMocHelpers::SlotData<bool()>(1, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'cleanOrphans'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'cleanPkgCache'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'systemUpdate'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'removeDBLock'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'tweaksSetupConnections'
        QtMocHelpers::SlotData<void(QStackedWidget *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QCheckBox *, QCheckBox *, QPushButton *)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 11 }, { 0x80000000 | 10, 12 }, { 0x80000000 | 10, 13 },
            { 0x80000000 | 10, 14 }, { 0x80000000 | 10, 15 }, { 0x80000000 | 10, 16 }, { 0x80000000 | 17, 18 },
            { 0x80000000 | 17, 19 }, { 0x80000000 | 10, 20 },
        }}),
        // Slot 'adaGamingMeta'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'removeAdaGamingMeta'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'adaDevelopmentMeta'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'removeAdaDevelopmentMeta'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'chaoticAUR'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'removeChaoticAUR'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'runCommand'
        QtMocHelpers::SlotData<bool(const QString &)>(27, 2, QMC::AccessPrivate, QMetaType::Bool, {{
            { QMetaType::QString, 28 },
        }}),
        // Slot 'checkChaoticAURStatus'
        QtMocHelpers::SlotData<int()>(29, 2, QMC::AccessPrivate, QMetaType::Int),
        // Slot 'backupPacmanConfig'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addVMware'
        QtMocHelpers::SlotData<void(QPushButton *)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 10, 32 },
        }}),
        // Slot 'vmwareStatus'
        QtMocHelpers::SlotData<bool()>(33, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'vmwareServiceStatus'
        QtMocHelpers::SlotData<bool()>(34, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'addonsSetupConnections'
        QtMocHelpers::SlotData<void(QStackedWidget *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QCheckBox *, QCheckBox *)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 36 }, { 0x80000000 | 10, 37 }, { 0x80000000 | 10, 38 },
            { 0x80000000 | 10, 39 }, { 0x80000000 | 10, 40 }, { 0x80000000 | 10, 32 }, { 0x80000000 | 17, 41 },
            { 0x80000000 | 17, 42 },
        }}),
        // Slot 'terminalSetupConnections'
        QtMocHelpers::SlotData<void(QStackedWidget *, QPushButton *, QPushButton *, QPushButton *, QPushButton *, QComboBox *, QLabel *)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 44 }, { 0x80000000 | 10, 45 }, { 0x80000000 | 10, 46 },
            { 0x80000000 | 10, 47 }, { 0x80000000 | 48, 49 }, { 0x80000000 | 50, 51 },
        }}),
        // Slot 'disableTermTheme'
        QtMocHelpers::SlotData<void(QPushButton *)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 10, 46 },
        }}),
        // Slot 'checkTermThemingStatus'
        QtMocHelpers::SlotData<int()>(53, 2, QMC::AccessPrivate, QMetaType::Int),
        // Slot 'mountDrivesSetupConnections'
        QtMocHelpers::SlotData<void(QStackedWidget *, QToolButton *)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { 0x80000000 | 55, 56 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Widget, qt_meta_tag_ZN6WidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6WidgetE_t>.metaTypes,
    nullptr
} };

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Widget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = _t->autostart();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 1: _t->cleanOrphans(); break;
        case 2: _t->cleanPkgCache(); break;
        case 3: _t->systemUpdate(); break;
        case 4: _t->removeDBLock(); break;
        case 5: _t->tweaksSetupConnections((*reinterpret_cast< std::add_pointer_t<QStackedWidget*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<QCheckBox*>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<QCheckBox*>>(_a[9])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[10]))); break;
        case 6: _t->adaGamingMeta(); break;
        case 7: _t->removeAdaGamingMeta(); break;
        case 8: _t->adaDevelopmentMeta(); break;
        case 9: _t->removeAdaDevelopmentMeta(); break;
        case 10: _t->chaoticAUR(); break;
        case 11: _t->removeChaoticAUR(); break;
        case 12: { bool _r = _t->runCommand((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 13: { int _r = _t->checkChaoticAURStatus();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->backupPacmanConfig(); break;
        case 15: _t->addVMware((*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[1]))); break;
        case 16: { bool _r = _t->vmwareStatus();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->vmwareServiceStatus();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->addonsSetupConnections((*reinterpret_cast< std::add_pointer_t<QStackedWidget*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<QCheckBox*>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<QCheckBox*>>(_a[9]))); break;
        case 19: _t->terminalSetupConnections((*reinterpret_cast< std::add_pointer_t<QStackedWidget*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<QComboBox*>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<QLabel*>>(_a[7]))); break;
        case 20: _t->disableTermTheme((*reinterpret_cast< std::add_pointer_t<QPushButton*>>(_a[1]))); break;
        case 21: { int _r = _t->checkTermThemingStatus();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 22: _t->mountDrivesSetupConnections((*reinterpret_cast< std::add_pointer_t<QStackedWidget*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QToolButton*>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 8:
            case 7:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QCheckBox* >(); break;
            case 9:
            case 6:
            case 5:
            case 4:
            case 3:
            case 2:
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QPushButton* >(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QStackedWidget* >(); break;
            }
            break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QPushButton* >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 8:
            case 7:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QCheckBox* >(); break;
            case 6:
            case 5:
            case 4:
            case 3:
            case 2:
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QPushButton* >(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QStackedWidget* >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 5:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QComboBox* >(); break;
            case 6:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QLabel* >(); break;
            case 4:
            case 3:
            case 2:
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QPushButton* >(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QStackedWidget* >(); break;
            }
            break;
        case 20:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QPushButton* >(); break;
            }
            break;
        case 22:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QStackedWidget* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QToolButton* >(); break;
            }
            break;
        }
    }
}

const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6WidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }
    return _id;
}
QT_WARNING_POP
