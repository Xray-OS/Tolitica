/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Widget_t {
    QByteArrayData data[17];
    char stringdata0[236];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 12), // "cleanOrphans"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 13), // "cleanPkgCache"
QT_MOC_LITERAL(4, 35, 12), // "systemUpdate"
QT_MOC_LITERAL(5, 48, 12), // "removeDBLock"
QT_MOC_LITERAL(6, 61, 16), // "setupConnections"
QT_MOC_LITERAL(7, 78, 15), // "QStackedWidget*"
QT_MOC_LITERAL(8, 94, 14), // "QStackedWidget"
QT_MOC_LITERAL(9, 109, 12), // "QPushButton*"
QT_MOC_LITERAL(10, 122, 12), // "tweaksButton"
QT_MOC_LITERAL(11, 135, 12), // "addonsButton"
QT_MOC_LITERAL(12, 148, 10), // "backButton"
QT_MOC_LITERAL(13, 159, 18), // "cleanOrphansButton"
QT_MOC_LITERAL(14, 178, 19), // "cleanPkgCacheButton"
QT_MOC_LITERAL(15, 198, 18), // "updateSystemButton"
QT_MOC_LITERAL(16, 217, 18) // "removeDBLockButton"

    },
    "Widget\0cleanOrphans\0\0cleanPkgCache\0"
    "systemUpdate\0removeDBLock\0setupConnections\0"
    "QStackedWidget*\0QStackedWidget\0"
    "QPushButton*\0tweaksButton\0addonsButton\0"
    "backButton\0cleanOrphansButton\0"
    "cleanPkgCacheButton\0updateSystemButton\0"
    "removeDBLockButton"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    8,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 9, 0x80000000 | 9, 0x80000000 | 9, 0x80000000 | 9, 0x80000000 | 9, 0x80000000 | 9, 0x80000000 | 9,    8,   10,   11,   12,   13,   14,   15,   16,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->cleanOrphans(); break;
        case 1: _t->cleanPkgCache(); break;
        case 2: _t->systemUpdate(); break;
        case 3: _t->removeDBLock(); break;
        case 4: _t->setupConnections((*reinterpret_cast< QStackedWidget*(*)>(_a[1])),(*reinterpret_cast< QPushButton*(*)>(_a[2])),(*reinterpret_cast< QPushButton*(*)>(_a[3])),(*reinterpret_cast< QPushButton*(*)>(_a[4])),(*reinterpret_cast< QPushButton*(*)>(_a[5])),(*reinterpret_cast< QPushButton*(*)>(_a[6])),(*reinterpret_cast< QPushButton*(*)>(_a[7])),(*reinterpret_cast< QPushButton*(*)>(_a[8]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 7:
            case 6:
            case 5:
            case 4:
            case 3:
            case 2:
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QPushButton* >(); break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QStackedWidget* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Widget.data,
    qt_meta_data_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Widget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
