#pragma once

#include <QAbstractListModel>

struct PackageInfo {
    QString name;
    QString version;
    QString description;
    QString repository;
    bool installed = false;
    QString installedVersion;
    QString size;
};

class PackageModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        VersionRole,
        DescriptionRole,
        RepositoryRole,
        InstalledRole,
        InstalledVersionRole,
        SizeRole,
    };
    Q_ENUM(Roles)

    explicit PackageModel(QObject *parent = nullptr);

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_packages.size(); }
    bool loading() const { return m_loading; }
    QString errorMessage() const { return m_errorMessage; }
    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    // Virtual interface for backends
    Q_INVOKABLE virtual void search(const QString &query) = 0;
    Q_INVOKABLE virtual void fetchDetails(int index) = 0;
    Q_INVOKABLE virtual void install(int index) = 0;
    Q_INVOKABLE virtual void remove(int index) = 0;

Q_SIGNALS:
    void countChanged();
    void loadingChanged();
    void errorMessageChanged();
    void searchQueryChanged();
    void detailsReady(int index, const QVariantMap &details);
    void confirmationRequired(const QString &action, const QString &packageName, const QString &message);
    void operationFinished(bool success, const QString &message);

protected:
    void setLoading(bool loading);
    void setError(const QString &msg);
    void clearError();
    void setPackages(const QList<PackageInfo> &packages);
    const QList<PackageInfo> &packages() const { return m_packages; }

private:
    QList<PackageInfo> m_packages;
    bool m_loading = false;
    QString m_errorMessage;
    QString m_searchQuery;
};
