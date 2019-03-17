class RawData{
	

	constructor(){
		this.arrayBuffer = new Object();
		this.dataView  = new Object();
		this.b64String = ""; 
	}
	
	fromBase64String(inb64){
		var binaryArray = Uint8Array.from(atob(document.getElementById("b64input").value), c => c.charCodeAt(0));
		this.arrayBuffer = binaryArray.buffer;
		this.dataView = new DataView(this.arrayBuffer);
		this.b64String = inb64;
	}

	setToEmpty(size){
		var emptyArray = new Uint8Array(size);
		this.arrayBuffer = emptyArray.buffer;
		this.dataView = new DataView(this.arrayBuffer);
		this.b64String = ""; 
	}	
	
	getB64String(){
		return this.b64String;
	}
	
	getArrayBuffer(){
		return this.arrayBuffer;
	}
	
	getRawData(){
		return new Uint8Array(this.arrayBuffer);
	}

	getSize(){
		return this.dataView.byteLength;
	}	

	getHexValue(offset, length){
		var ret = "";
		var wrkArr = new Uint8Array(this.arrayBuffer);
		wrkArr.slice(offset, offset + length).forEach(function(e){
			ret += byteToHexString(e) + " ";
		});
		return ret;

		function byteToHexString(inUint8){
			var ret = inUint8.toString(16).toUpperCase();
			if (ret.length % 2) {
				ret = '0' + ret;
			}
			return ret;
		}	
	}	

	encodeToB64(){
		this.b64String = btoa(String.fromCharCode.apply(null, this.getRawData()));
	}
	
	getUint32(offset){
		return this.dataView.getUint32(offset, true);
	}

	setUint32(offset, inValue){
		this.dataView.setUint32(offset, inValue, true);
	}

	getUint16(offset){
		return this.dataView.getUint16(offset, true);
	}

	setUint16(offset, inValue){
		this.dataView.setUint16(offset, inValue, true);
	}

	getUint8(offset){
		return this.dataView.getUint8(offset);
	}

	setUint8(offset, inValue){
		this.dataView.setUint8(offset, inValue);
	}
	
	getInt32(offset){
		return this.dataView.getInt32(offset, true);
	}

	setInt32(offset, inValue){
		this.dataView.setInt32(offset, inValue, true);
	}
	
	getInt16(offset){
		return this.dataView.getInt16(offset, true);
	}

	setInt16(offset, inValue){
		this.dataView.setInt16(offset, inValue, true);
	}

	getInt8(offset){
		return this.dataView.getInt8(offset);
	}
	
	setInt8(offset, inValue){
		this.dataView.setInt8(offset, inValue);
	}

	getFloat32(offset){
		return this.dataView.getFloat32(offset, true).toPrecision(4);
	}

	setFloat32(offset, inValue){
		this.dataView.setFloat32(offset, inValue, true);
	}

	getString(offset, length){
		var wrkArr = new Uint8Array(this.arrayBuffer, offset, length).filter(c => c > 0);
		if (wrkArr.length > 0){
			return String.fromCharCode.apply(null, wrkArr);
		}
		else{
			return "";
		}
	}

	setString(offset, inValue, length){
		inValue = inValue.replace(/\s+$/g, ''); // rtrim
		inValue.padEnd(String.fromCharCode(0), length); // fill with \0
		
		if (inValue.length > length){
			alert("String >" + inValue + "< will be truncated to " + length + " character");
			inValue = inValue.substring(0,length);
		}
		for (var i = 0; i < length; i++){
			this.dataView.setUint8(offset + i, inValue.charCodeAt(i));
		}	
	}
	
	getUint64(offset){
		const left =  this.getUint32(offset);
		const right = this.getUint32(offset + 4);
		const combined = left + 2**32*right;

		if (!Number.isSafeInteger(combined)){
			console.warn(combined, 'exceeds MAX_SAFE_INTEGER. Precision may be lost');
		}
		return combined;
	}

	setUint64(offset, inValue){
		const left = inValue % (2**32); 
		const right = Math.floor(inValue / (2**32));
		
		this.setUint32(offset, left);
		this.setUint32(offset + 4);
	}
	
}


class MemoryMapFactory{	
	cppSrc2DescriptorJson(inText){
		var out = new Object();
		inText = inText
			.replace(/\/\*.+?\*\/|\/\/.*(?=[\n\r])/g  ,"") // remove comments
			.replace(/=.*(?=;)/g,"") // remove after =
			.replace(/\n/g, "") // remove line break
			.replace(/\t/g, " ") // tab -> space
			.replace(/ {2,}/g," "); // spaces -> one space

		var structTextArr = inText.match(/\bstruct\b.*?{.*?}/g);  
		var structArr = [];
		
		structTextArr.forEach(function(structText){
			var structName = structText.match(/struct(.*){/)[1].trim();
			structArr = out[structName] = [];
			structText.match(/struct.*{(.*)}/)[1].split(";").filter(l => l.trim() != "").forEach(function(e){
				var exprArr = e.trim().split(" ");
				if (exprArr.length == 2){
					structArr.push( {"dataType": exprArr[0], "name": exprArr[1]} );
				}
				else{
					console.error("Parse error: " + e);
				}				
			});
		});
		
		return JSON.stringify(out, null, 3);	  
	}

	createFromDescriptorJson(structDescriptorJson){
		var structDescriptorObj = JSON.parse(structDescriptorJson);
		var wrkMemoryMap = {};
		var oMemoryMapElements = [];
		var iStructures = Object.getOwnPropertyNames(structDescriptorObj); 
		iStructures.forEach(function(iStructName){ 
			var offset = 0;
			var maximalPadding = 0;
			var iStructElements = structDescriptorObj[iStructName];
			wrkMemoryMap[iStructName] = {};
			wrkMemoryMap[iStructName]["elements"] = [];
			oMemoryMapElements = wrkMemoryMap[iStructName].elements;
			iStructElements.forEach(function(iStructElement){
				charArrayPreprocessor(iStructElement);
				
				var typeDef = typeDefArr[iStructElement.dataType];	
				var attributeInfo = getAttributeInfo(iStructElement.name);

				if (typeDef != undefined){ // native type
					if (parseInt(typeDef.bytePadding) > maximalPadding){
						maximalPadding = parseInt(typeDef.bytePadding);
					}
			
					attributeInfo.occurenceList.forEach(function(occ){
						offset += parseInt(calcPadding(offset, typeDef.bytePadding));
						oMemoryMapElements.push({"dataType": iStructElement.dataType, "reference": attributeInfo.name + occ, "offset": offset, "size": parseInt(typeDef.byteSize)});
						offset += parseInt(typeDef.byteSize);					
					});
				}
				else{  // inner struct
					var innerStruct = wrkMemoryMap[iStructElement.dataType];
					
					if (innerStruct == undefined){
						alert("Error: '" + iStructElement.dataType + "' is not a native data type and not found in structure definition.");
						return;
					}
					
					offset += calcPadding(offset, innerStruct.paddingBefore);
					
					
					attributeInfo.occurenceList.forEach(function(occ){
						var startOffset = offset;				
						var referencePrefix = attributeInfo.name + occ;
						innerStruct.elements.forEach(function(innerStructElement){
							offset = startOffset + innerStructElement.offset;
							oMemoryMapElements.push({"dataType": innerStructElement.dataType, "reference": referencePrefix + "." + innerStructElement.reference, "offset": offset, "size": innerStructElement.size});

						});							
						offset += parseInt(typeDefArr[innerStruct["elements"][innerStruct.elements.length - 1]["dataType"]].byteSize);				
						offset += calcPadding(offset, innerStruct.paddingAfter);
					});		
				}
			});	
			
			wrkMemoryMap[iStructName].paddingBefore = maximalPadding;
			wrkMemoryMap[iStructName].paddingAfter = maximalPadding;
		});
		
		var stName = iStructures[iStructures.length-1];
		
		var structMemoryMap = new Object();
		structMemoryMap.elements = wrkMemoryMap[stName].elements;

		var lastElement = structMemoryMap.elements[structMemoryMap.elements.length-1];
		structMemoryMap.size = wrkMemoryMap[stName].size = lastElement.offset + lastElement.size; 

		//console.log(JSON.stringify(wrkMemoryMap));
		return structMemoryMap;
		
		function charArrayPreprocessor(inStructElement){
			var bracketPos = inStructElement.name.lastIndexOf("[");
			if ((inStructElement.dataType == "char") && (bracketPos != -1)){
				var newName = inStructElement.name.substring(0, bracketPos);
				var size = inStructElement.name.substring(bracketPos + 1, inStructElement.name.lastIndexOf("]"));
				var newDataType = "char[" + size + "]";
				
				if (typeDefArr[newDataType] == undefined){
					typeDefArr[newDataType] = {"byteSize": size, "bytePadding": 1, "getMethod": "getString", "setMethod": "setString"};		
				}

				inStructElement.name = newName;
				inStructElement.dataType = newDataType;
			}
		}

		function getAttributeInfo(attributeName){
			var ret = {};
			var bracketPos = attributeName.indexOf("[");
			
			if (bracketPos == -1 ){
				ret.name = attributeName;
				ret.occurence = 1;
				ret.occurenceList = [];
				ret.occurenceList[0] = "";
			}
			else{
				ret.name = attributeName.substring(0, bracketPos);
				ret.occurenceList = getDimensionList(attributeName.substring(bracketPos + 1));
				ret.occurence = ret.occurenceList.length;
			}
			return ret;


			function getDimensionList(dimensionsPart){
				dimensionsPart = dimensionsPart.replace(/\]/g, "");
				var dimensionsArr = dimensionsPart.split("[").map(x => parseInt(x));
				var occurence = dimensionsArr.reduce( (acc, x) => acc * x);
				var calcArr = [];
				
				for (i = 0; i < dimensionsArr.length-1; i++){
					var val = 1;
					for (j = i + 1; j < dimensionsArr.length; j++){
						val = val * dimensionsArr[j];
					}
					calcArr.push(val);
				}
				calcArr.push(1);
				
				var retArr = [];
				
				for (var i = 0; i < occurence; i++){
					var e = "";
					var x = i;
					for(var j = 0; j < calcArr.length; j++){ 
						e = e + "[" + Math.floor(x / calcArr[j]) + "]";
						x = x % calcArr[j];
					}
					retArr.push(e);
				}
				
				return retArr;
			}
		}
		
		function calcPadding(offset, padding){
			return parseInt( (parseInt(offset * -1) & parseInt(padding - 1)) );
		}	
	}

	descriptorJson2CppSrc(structDescriptorJson){
		var structDescriptorObj = JSON.parse(structDescriptorJson);
		var out = "";
		var crlf = "\r\n";
		var tab = "   ";
		var iStructures = Object.getOwnPropertyNames(structDescriptorObj); 
		iStructures.forEach(function(iStructName){ 
			var iStructElements = structDescriptorObj[iStructName];
			out += "struct " + iStructName + "{" + crlf;
			iStructElements.forEach(function(iStructElement){
				out += tab + iStructElement.dataType + " " + iStructElement.name + ";" + crlf;
			});
			out += "};" + crlf + crlf;
		});
		return out;
	}	
}

class CppStructJsonConverter{
	constructor(){
		this.memoryMap = null;
	}

	getMemoryMapTable(rawData){
		if (rawData.getSize() != this.memoryMap.size){
			alert("ERROR! Memory dump size: " + rawData.getSize() + ", Structure descriptor calculated size: " + this.memoryMap.size);
		}
		
		var tbl = document.createElement("table");
		tbl.setAttribute("id", "memory-map-table")
		var tblHead = document.createElement("thead");
		var tblBody = document.createElement("tbody");

		createRowCells(tblHead, new Array("offset", "reference", "data type", "bytes", "hex content", "content"));
		tbl.appendChild(tblHead);

		this.memoryMap.elements.forEach(function(element, idx){  
			if (idx > 0){
				var padding = element.offset - (this.memoryMap.elements[idx-1].offset + this.memoryMap.elements[idx-1].size);
				if (padding > 0){
					var r = createRowCells(tblBody, new Array(parseInt(element.offset - padding), "|PADDING|", "", padding, rawData.getHexValue(parseInt(element.offset - padding), padding), ""));
					r.className = "memory-map-padding";
				}
			}

			var hexOutValue = "";
			var outValue = "";
			var wasError = false;
			try{
				hexOutValue =  rawData.getHexValue(element.offset, element.size);
				outValue =  rawData[typeDefArr[element.dataType].getMethod](element.offset, element.size);
			}
			catch (e) {
				wasError = true;
				console.error(e);
			}	

			var r = createRowCells(tblBody, new Array(element.offset, element.reference, element.dataType, element.size, hexOutValue, outValue) ); 
			if (wasError){
				r.className = "memory-map-error";
			}			

		}, this);

		tbl.appendChild(tblBody);
		
		return tbl;
		
		function createCell(inRow, cellText){
			var cell = document.createElement("td");
			cellText = document.createTextNode(cellText);
			cell.appendChild(cellText);
			inRow.appendChild(cell)
		}
		
		function createRowCells(rowParent, inCells){
			var row = document.createElement("tr");
			inCells.forEach(function(cell){
				createCell(row, cell);
			});
			rowParent.appendChild(row);
			return row;
		}
	}  	
	
	raw2json(rawData){
		var outputDataJson = new Object();
		this.memoryMap.elements.forEach(function(structMemoryMapEntry) {
			var propertyNames = structMemoryMapEntry.reference.replace(/]/g, '').replace(/\[/g, '[].').split('.');
			var o = outputDataJson;
			for (var j = 0; j < propertyNames.length; j++){
				var propertyInfo = getPropertyInfo(propertyNames, j);
				//console.log(propertyInfo);
				//console.log(outputDataJson);

				if (!(propertyInfo.basePropertyName in o)) {
					if (j < propertyNames.length - 1){
						o[propertyInfo.basePropertyName] = propertyInfo.defaultValue;
					}
					else{		
						o[propertyInfo.basePropertyName] = rawData[typeDefArr[structMemoryMapEntry.dataType].getMethod](structMemoryMapEntry.offset, structMemoryMapEntry.size);
					}	
				}
				o = o[propertyInfo.basePropertyName];
			}		
		}, this);

		return JSON.stringify(outputDataJson, null, 3);
		
		function getPropertyInfo(propArr, idx){
			var propertyInfo = {};
			var bracketPos = propArr[idx].indexOf("[");
			
			if (bracketPos == -1 ){
				propertyInfo.basePropertyName = propArr[idx];
				if (idx == propArr.length-1){
					propertyInfo.defaultValue = "";
				}
				else{
					propertyInfo.defaultValue = {};
				}
			}
			else{
				propertyInfo.basePropertyName = propArr[idx].substring(0, bracketPos);
				propertyInfo.defaultValue = [];
			}
			return propertyInfo;
		}	
	}	
	
	json2raw(dataJson){
		var dataObj = JSON.parse(dataJson);
		var rawData = new RawData();
		rawData.setToEmpty(this.memoryMap.size);
		this.memoryMap.elements.forEach(function(element){
			var value = Object.getByString(dataObj, element.reference);
			var setterMethod = typeDefArr[element.dataType].setMethod;
			if (value != undefined){
				rawData[setterMethod](element.offset, value, element.size);
			}
			else{
				alert(element.reference + "not found");
			}
		}, this);
		return rawData;
	}
}

const typeDefArr = {
	"uint8_t" :	{
					"byteSize" : "1",
					"bytePadding" : "1",
					"getMethod" : "getUint8",
					"setMethod" : "setUint8"
				},
	"int8_t" : 	{
					"byteSize" : "1",
					"bytePadding" : "1",
					"getMethod" : "getInt8",
					"setMethod" : "setInt8"
				},
	"uint16_t" :{
					"byteSize" : "2",
					"bytePadding" : "2",
					"getMethod" : "getUint16",
					"setMethod" : "setUint16"
				},
	"int16_t" :	{
					"byteSize" : "2",
					"bytePadding" : "2",
					"getMethod" : "getInt16",
					"setMethod" : "setInt16"
				},
	"uint32_t" :{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getUint32",
					"setMethod" : "setUint32"
				},
	"int32_t" :	{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getInt32",
					"setMethod" : "setInt32"
				},
	"float32" : 	{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getFloat32",
					"setMethod" : "setFloat32"
				},
	"float" : 	{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getFloat32",
					"setMethod" : "setFloat32"
				},
	"char" : 	{
					"byteSize" : "1",
					"bytePadding" : "1",
					"getMethod" : "getString",
					"setMethod" : "setString"
				},
	"int" :		{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getInt16",
					"setMethod" : "setInt16"
				},				
	"uint" :		{
					"byteSize" : "4",
					"bytePadding" : "4",
					"getMethod" : "getUint16",
					"setMethod" : "setUint16"
				},				
	"uint64_t" :	{
					"byteSize" : "8",
					"bytePadding" : "8",
					"getMethod" : "getUint64",
					"setMethod" : "setUint64"
				},
	"byte" :	{
					"byteSize" : "1",
					"bytePadding" : "1",
					"getMethod" : "getUint8",
					"setMethod" : "setUint8"
				}
				
};
/*	 new types in runtime:
		"char[xx]":{
						"byteSize" : "xx",
						"bytePadding" : "1",
						"getMethod" : "getCharArray",
						"setMethod" : "setCharArray"
					}					
	where xx is the actual char array size

*/	


Object.getByString = function(o, s) {
	s = s.replace(/\[(\w+)\]/g, '.$1'); 
	s = s.replace(/^\./, '');   
	var a = s.split('.');
	for (var i = 0, n = a.length; i < n; ++i) {
		var k = a[i];
		if (k in o) {
			o = o[k];
		} else {
			return;
		}
	}
	return o;
}

function ajaxHttpGet(url, callback) {
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
		   if (xmlhttp.status == 200) {
			   callback(xmlhttp.responseText);
		   }
		   else if (xmlhttp.status == 400) {
			  alert('There was an error 400');
		   }
		   else {
			   alert('something else other than 200 was returned');
		   }
		}
	};

	xmlhttp.open("GET", url, true);
	xmlhttp.send();
}


function ajaxHttpPost(url, params, callback) { 
	var xmlhttp = new XMLHttpRequest();

	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
		   if (xmlhttp.status == 200) {
			   callback(xmlhttp.responseText);
		   }
		   else if (xmlhttp.status == 400) {
			  alert('There was an error 400');
		   }
		   else {
			   alert('something else other than 200 was returned');
		   }
		}
	};

	xmlhttp.open("POST", url, true);
	xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlhttp.send(params);
}


function readFile(e, callback) {
	var file = e.target.files[0];
	if (!file) {	
		return;
	}
	var reader = new FileReader();
	reader.onload = function(e) {
		callback(e.target.result);
	};
	reader.readAsText(file);
}