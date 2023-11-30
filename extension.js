// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');
const fs = require('fs');
const { exec } = require('child_process');

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed

const oneTab = "    ";

function isSpace(c){
	return (c == ' ');
}

function isEndOfLine(c){
	return (c == '\n') || (c == '\r') || (c == '\r\n');
}

function isNewEndOfLine(c){
	return (c == ';') || (c == '{') || (c == '}');
}

//УДАЛЕНИЕ ЛИШНИХ ПРОБЕЛОВ
function removeExtraSpaceSymbols(countOfSymbols, text, ignore){
	for (let currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition){
		if(ignore[currentPosition] == true) {
			continue;
		}
		if(currentPosition == 0 || isNewEndOfLine(text.charAt(currentPosition))){
			if (currentPosition == 0 && isSpace(text.charAt(currentPosition))) {
                ignore[currentPosition] = true;
            }
			while(currentPosition + 1 < countOfSymbols && isSpace(text.charAt(currentPosition + 1))){
				++currentPosition;
				ignore[currentPosition] = true;
			}
		}
		if(currentPosition >= countOfSymbols) {
			break;
		}
		if(isSpace(text.charAt(currentPosition))){
			while(currentPosition + 1 < countOfSymbols && isSpace(text.charAt(currentPosition + 1))){
				++currentPosition;
				ignore[currentPosition] = true;
			}
		}
		if(currentPosition >= countOfSymbols) {
			break;
		}
		if(isEndOfLine(text.charAt(currentPosition))){
			while(currentPosition + 1 < countOfSymbols && isSpace(text.charAt(currentPosition + 1))){
				++currentPosition;
				ignore[currentPosition] = true;
			}
		}
		if(currentPosition >= countOfSymbols) {
			break;
		}
		if(currentPosition + 1 < countOfSymbols && (text.charAt(currentPosition) == '/' && text.charAt(currentPosition + 1) == '/')){
			while(currentPosition - 1 >= 0 && isSpace(text.charAt(currentPosition - 1)) && !ignore[currentPosition - 1]){
				--currentPosition;
				ignore[currentPosition] = true;
			}
		}
	}
}

//УДАЛЕНИЕ ИГНОРИРУЕМЫХ СИМВОЛОВ
function refactorText(countOfSymbols, text, ignore, immutableSymbols){
	let charArray = [];
	for (let currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition){
		if(!ignore[currentPosition] || immutableSymbols[currentPosition]) {
			charArray.push(text.charAt(currentPosition));
		}
	}
	let newText = charArray.join('');
	return newText;
}

function setImmutableSymbols(countOfSymbols, text, immutableSymbols){
	for (let currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition){
		if(text.charCodeAt(currentPosition) == 34){
			while(currentPosition + 1 < countOfSymbols && text.charCodeAt(currentPosition + 1) != 34){
				immutableSymbols[currentPosition] = true;
				++currentPosition;
			}
			++currentPosition;
			if(currentPosition < countOfSymbols){
				immutableSymbols[currentPosition] = true;
			}
		}
		if(currentPosition + 1 < countOfSymbols && text.charAt(currentPosition) == '/'){
			if(text.charAt(currentPosition + 1) == '/'){
				while(currentPosition + 1 < countOfSymbols && !isEndOfLine(text.charAt(currentPosition + 1))){
					immutableSymbols[currentPosition] = true;
					++currentPosition;
				}
			}
			else if(text.charAt(currentPosition + 1) == '*'){
				while(currentPosition + 2 < countOfSymbols && !(text.charAt(currentPosition + 1) == '*' && text.charAt(currentPosition + 2) == '/')){
					immutableSymbols[currentPosition] = true;
					++currentPosition;
				}
			}
		}
	}
}

//РАЗБИЕНИЕ ТЕКСТА НА СТРОКИ
function partitionTextToLines(countOfSymbols, text, immutableSymbols, stringsPartition){
	let currentStringNumber = 0;
	for (let currentPosition = 0; currentPosition < countOfSymbols; ++currentPosition){
		if(currentStringNumber >= stringsPartition.length){
			stringsPartition.push([]);
		}
		if(immutableSymbols[currentPosition]){
			stringsPartition[currentStringNumber].push(text.charAt(currentPosition));
		}
		else if(isNewEndOfLine(text.charAt(currentPosition))){
			if(currentPosition + 1 < countOfSymbols && text.charAt(currentPosition + 1) == '/'){
				stringsPartition[currentStringNumber].push(text.charAt(currentPosition));
				++currentPosition;
				//Добавить 3 таба перед комментарием
				for (let i = 0; i < 3; ++i){
					stringsPartition[currentStringNumber].push(oneTab);
				}
				while(currentPosition < countOfSymbols && !isEndOfLine(text.charAt(currentPosition))){
					stringsPartition[currentStringNumber].push(text.charAt(currentPosition));
					++currentPosition;
				}
				stringsPartition[currentStringNumber].push('\n');
				++currentStringNumber;
			}
			else {
				stringsPartition[currentStringNumber].push(text.charAt(currentPosition));
				stringsPartition[currentStringNumber].push('\n');
				if(currentPosition + 1 < countOfSymbols && isEndOfLine(text.charAt(currentPosition + 1))){
					++currentPosition;
				}
				++currentStringNumber;
			}
		}
		else if(isEndOfLine(text.charAt(currentPosition))){
			stringsPartition[currentStringNumber].push('\n');
			++currentStringNumber;
		}
		else {
			stringsPartition[currentStringNumber].push(text.charAt(currentPosition));
		}
	}
}

//ОПРЕДЕЛЕНИЕ УРОВНЯ ВЛОЖЕННОСТИ
function setLevelOfNesting(countOfStrings, stringsPartition, setLevelOfNesting){
	let currentLevel = 0;
	for(let currentStringNumber = 0; currentStringNumber < countOfStrings; ++currentStringNumber){
		setLevelOfNesting[currentStringNumber] = currentLevel;
		let localBalance = 0;
		let localCountOfSymbols = stringsPartition[currentStringNumber].length;
		for (let currentPosition = 0; currentPosition < localCountOfSymbols; ++currentPosition){
			if(stringsPartition[currentStringNumber][currentPosition] == '{'){
				++currentLevel;
				++localBalance;
			}
			else if (stringsPartition[currentStringNumber][currentPosition] == '}') {
				--currentLevel;
				--localBalance;
				if(localBalance < 0){
					--setLevelOfNesting[currentStringNumber];
				}
			}
		}
	}
}


function mainFunc(text){
	let countOfSymbols = text.length;
	let ignore = new Array(countOfSymbols).fill(false);
	let immutableSymbols = new Array(countOfSymbols).fill(false);

	removeExtraSpaceSymbols(countOfSymbols, text, ignore);
	setImmutableSymbols(countOfSymbols, text, immutableSymbols);
	text = refactorText(countOfSymbols, text, ignore, immutableSymbols);
	countOfSymbols = text.length;
	ignore = new Array(countOfSymbols).fill(false);
	immutableSymbols = new Array(countOfSymbols).fill(false);

	let stringsPartition = [[]];
	setImmutableSymbols(countOfSymbols, text, immutableSymbols);
	partitionTextToLines(countOfSymbols, text, immutableSymbols, stringsPartition);
	
	let countOfStrings = stringsPartition.length;
	let LevelOfNesting = new Array(countOfStrings).fill(0);
	setLevelOfNesting(countOfStrings, stringsPartition, LevelOfNesting);

	let newTextArray = [];
	for (let currentStringNumber = 0; currentStringNumber < countOfStrings; ++currentStringNumber){
		newTextArray.push([]);
		for (let i = 0; i < LevelOfNesting[currentStringNumber]; ++i){
			newTextArray.push(oneTab);
		}
		newTextArray.push(stringsPartition[currentStringNumber].join(''));
	}
	text = newTextArray.join('');
	return text;
}

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {

	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated
	console.log('Congratulations, your extension "tabulationPlugin" is now active!');

	let disposable = vscode.commands.registerCommand('tabulationPlugin.refactorCode', async function () {
		console.log('command activated');
		try {
			const editor = vscode.window.activeTextEditor;
			if (!editor) {
				vscode.window.showErrorMessage('No active text editor');
				return;
			}

			const document = editor.document;
			const fullText = document.getText();

			// Замена кода в редакторе на считанный текст
			let newText = mainFunc(fullText);

			editor.edit(editBuilder => {
				const documentStart = new vscode.Position(0, 0);
				const documentEnd = new vscode.Position(document.lineCount - 1, document.lineAt(document.lineCount - 1).text.length);
				const fullRange = new vscode.Range(documentStart, documentEnd);
				editBuilder.replace(fullRange, newText);
			});
			
		} catch (error) {
			vscode.window.showErrorMessage(`Error: ${error.message}`);
			console.log(`Error: ${error.message}`);
		}
	});

	context.subscriptions.push(disposable);


	
	
}

// This method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
