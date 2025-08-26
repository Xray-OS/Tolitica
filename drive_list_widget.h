#ifndef DRIVE_LIST_WIDGET_H
#define DRIVE_LIST_WIDGET_H

#include <QWidget>
#include <QTreeWidget>

class drive_list_widget : public QWidget
{
    Q_OBJECT
public:
    explicit drive_list_widget(QWidget *parent = nullptr);

    // Public method to refresh the drive list.
    void refresh();

    // Goes through all items, writes the new config files, and calls the mount/unmount helper.
    // Returns true on sucess.
    bool applyMountSelection();
    bool operationCancelled() const { return m_operationCancelled; }

    // Opens a dialog to let the user choose additional partitions to show.
    void showAdditionalPartitionsDialog();

    bool hasDangerousChange() const {return m_dangerousChange;}
    void resetDangerousChange() { m_dangerousChange = false; }

    // Checks if any checkbox has been modified relative to its default state.
    bool isModified() const;
    bool isDangerousModified() const;

    // Reads the configuration file and returns a QSet containing all enabled device UUID
    // tokens (ignoring empty lines or comments).
    QSet<QString> loadManuallyEnabledDevices() const;

signals:
    // Emitted whenever a checkbox (drive or partition) is toggled.
    void selectionChanged();

private:
    QTreeWidget *m_treeWidget;

    // User options to display additional partitions
    bool m_showSwap = false; // By default we hide swap partitions.
    bool m_showBoot = false; // By default we hide bbot partitions.
    bool m_showHidden = false; // By default we hide hidden partitions.

    bool m_ignoreSelectionChanges = false;
    bool m_dangerousChange = false;

    // Operation canceled don't return error
    bool m_operationCancelled = false;
signals:
};

#endif // DRIVE_LIST_WIDGET_H
