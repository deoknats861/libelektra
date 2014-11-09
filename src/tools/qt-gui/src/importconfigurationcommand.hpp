#ifndef IMPORTCONFIGURATIONCOMMAND_H
#define IMPORTCONFIGURATIONCOMMAND_H

#include <QUndoCommand>
#include <kdb.hpp>
#include "treeviewmodel.hpp"

class ImportConfigurationCommand : public QUndoCommand
{

public:
	explicit ImportConfigurationCommand(TreeViewModel* model, const QString keyName, const QString format, const QString file, const QString mergeStrategy, QUndoCommand* parent = 0);

	virtual void undo();
	virtual void redo();

private:

	TreeViewModel* m_model;
	QString m_name;
	QString m_format;
	QString m_file;
	QString m_mergeStrategy;
	kdb::KDB m_kdb;
	kdb::KeySet m_set;

};

#endif // IMPORTCONFIGURATIONCOMMAND_H
