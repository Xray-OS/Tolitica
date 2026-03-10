/****************************************************************************
** Meta object code from reading C++ file 'core_initial.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../core_initial.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'core_initial.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN11CoreInitialE_t {};
} // unnamed namespace

template <> constexpr inline auto CoreInitial::qt_create_metaobjectdata<qt_meta_tag_ZN11CoreInitialE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CoreInitial",
        "themeApplied",
        "",
        "themeId",
        "reloadFinished",
        "applyGlobalTheme",
        "osreleaseStatus",
        "setOSrelease",
        "konsoleProfStatus",
        "setKonsoleProfile",
        "reloadPlasmaByReplace",
        "reloadPlasmaByDBus",
        "themeStatus",
        "xrayThemeStatus",
        "grubThemeStatus",
        "currentGrubTheme",
        "listGrubThemes",
        "setGrubTheme",
        "grubTheme",
        "currentIcons",
        "setIcons",
        "icons",
        "aurStatus",
        "aurHelper",
        "getRemoveAUR",
        "QWidget*",
        "parent",
        "std::function<void(bool)>",
        "callback",
        "storeStatus",
        "store",
        "getRemoveStore",
        "gamingMetaStatus",
        "getXrayGamingMeta"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'themeApplied'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'reloadFinished'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'applyGlobalTheme'
        QtMocHelpers::SlotData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'osreleaseStatus'
        QtMocHelpers::SlotData<bool()>(6, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'setOSrelease'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'konsoleProfStatus'
        QtMocHelpers::SlotData<bool()>(8, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'setKonsoleProfile'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'reloadPlasmaByReplace'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'reloadPlasmaByDBus'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'themeStatus'
        QtMocHelpers::SlotData<bool()>(12, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'xrayThemeStatus'
        QtMocHelpers::SlotData<bool()>(13, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'grubThemeStatus'
        QtMocHelpers::SlotData<bool()>(14, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'currentGrubTheme'
        QtMocHelpers::SlotData<QString()>(15, 2, QMC::AccessPublic, QMetaType::QString),
        // Slot 'listGrubThemes'
        QtMocHelpers::SlotData<QStringList()>(16, 2, QMC::AccessPublic, QMetaType::QStringList),
        // Slot 'setGrubTheme'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Slot 'currentIcons'
        QtMocHelpers::SlotData<QString()>(19, 2, QMC::AccessPublic, QMetaType::QString),
        // Slot 'setIcons'
        QtMocHelpers::SlotData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Slot 'aurStatus'
        QtMocHelpers::SlotData<bool(const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 23 },
        }}),
        // Slot 'getRemoveAUR'
        QtMocHelpers::SlotData<void(QWidget *, const QString &, std::function<void(bool)>)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { QMetaType::QString, 23 }, { 0x80000000 | 27, 28 },
        }}),
        // Slot 'getRemoveAUR'
        QtMocHelpers::SlotData<void(QWidget *, const QString &)>(24, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { QMetaType::QString, 23 },
        }}),
        // Slot 'storeStatus'
        QtMocHelpers::SlotData<bool(const QString &)>(29, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 30 },
        }}),
        // Slot 'getRemoveStore'
        QtMocHelpers::SlotData<void(QWidget *, const QString &, std::function<void(bool)>)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { QMetaType::QString, 30 }, { 0x80000000 | 27, 28 },
        }}),
        // Slot 'getRemoveStore'
        QtMocHelpers::SlotData<void(QWidget *, const QString &)>(31, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { QMetaType::QString, 30 },
        }}),
        // Slot 'gamingMetaStatus'
        QtMocHelpers::SlotData<bool()>(32, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'getXrayGamingMeta'
        QtMocHelpers::SlotData<void(QWidget *, std::function<void(bool)>)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { 0x80000000 | 27, 28 },
        }}),
        // Slot 'getXrayGamingMeta'
        QtMocHelpers::SlotData<void(QWidget *)>(33, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CoreInitial, qt_meta_tag_ZN11CoreInitialE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CoreInitial::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreInitialE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreInitialE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11CoreInitialE_t>.metaTypes,
    nullptr
} };

void CoreInitial::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CoreInitial *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->themeApplied((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->reloadFinished(); break;
        case 2: _t->applyGlobalTheme((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: { bool _r = _t->osreleaseStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->setOSrelease(); break;
        case 5: { bool _r = _t->konsoleProfStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 6: _t->setKonsoleProfile(); break;
        case 7: _t->reloadPlasmaByReplace(); break;
        case 8: _t->reloadPlasmaByDBus(); break;
        case 9: { bool _r = _t->themeStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->xrayThemeStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 11: { bool _r = _t->grubThemeStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { QString _r = _t->currentGrubTheme();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 13: { QStringList _r = _t->listGrubThemes();
            if (_a[0]) *reinterpret_cast<QStringList*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->setGrubTheme((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: { QString _r = _t->currentIcons();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 16: _t->setIcons((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: { bool _r = _t->aurStatus((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->getRemoveAUR((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<std::function<void(bool)>>>(_a[3]))); break;
        case 19: _t->getRemoveAUR((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: { bool _r = _t->storeStatus((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 21: _t->getRemoveStore((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<std::function<void(bool)>>>(_a[3]))); break;
        case 22: _t->getRemoveStore((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 23: { bool _r = _t->gamingMetaStatus();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 24: _t->getXrayGamingMeta((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<std::function<void(bool)>>>(_a[2]))); break;
        case 25: _t->getXrayGamingMeta((*reinterpret_cast<std::add_pointer_t<QWidget*>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        case 21:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        case 22:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        case 24:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        case 25:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QWidget* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CoreInitial::*)(const QString & )>(_a, &CoreInitial::themeApplied, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CoreInitial::*)()>(_a, &CoreInitial::reloadFinished, 1))
            return;
    }
}

const QMetaObject *CoreInitial::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CoreInitial::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreInitialE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CoreInitial::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void CoreInitial::themeApplied(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void CoreInitial::reloadFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
