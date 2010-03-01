#import <Foundation/Foundation.h>
#import "../../base/Tools/AGSParser.h"


@interface NSMutableArray (AddNumber)
/**
 * Convenience method for adding a number to an array.
 */
- (void)addNumber: (NSInteger)number;
@end
@implementation NSMutableArray (AddNumber)
- (void)addNumber: (NSInteger)number
{
	[self addObject: [NSNumber numberWithInteger: number]];
}
@end

@interface NSDictionary (ObjectOrKey)
/**
 * Returns the object for the key if one exists, or the key if one doesn't.
 */
- (id)objectOrKey: (id)aKey;
@end
@implementation NSDictionary (ObjectOrKey)
- (id)objectOrKey: (id)aKey
{
	id object = [self objectForKey: aKey];
	return (nil != object) ? object : aKey;
}
@end
@interface NSFileHandle (WriteString)
/**
 * Writes the format string, UTF-8 encoded, to the file.
 */
- (void)writeFormat: (NSString*)fmt, ...;
/**
 * Returns a file handle for writing to a new file.
 */
+ (NSFileHandle*)fileHandleForWritingNewFileAtPath: (NSString*)aPath;
@end
@implementation NSFileHandle (WriteString)
- (void)writeFormat: (NSString*)fmt, ...
{
	va_list ap;
	va_start(ap, fmt);
	NSString *str = [[NSString alloc] initWithFormat: fmt
	                                       arguments: ap];
	id pool = [NSAutoreleasePool new];
	[self writeData: [str dataUsingEncoding: NSUTF8StringEncoding]];
	[pool release];
	[str release];
}
+ (NSFileHandle*)fileHandleForWritingNewFileAtPath: (NSString*)aPath
{
	NSData *data = [NSData data];
	[data writeToFile: aPath atomically: NO];
	return [self fileHandleForWritingAtPath: aPath];
}
@end

/**
 * Mappings from Objective-C to C type names.
 */
NSDictionary *typeMappings;

NSString *classToType(NSString* className)
{
	return [className stringByReplacingOccurrencesOfString: @"NS"
	                                            withString: @"CF"];
}

/**
 * Parse a header and dump a .plist file for each method with the guesses at
 * what the C versions should be.
 */
NSArray *parseHeader(NSString *header)
{
	AGSParser *parser = [AGSParser new];
	// Needed because AGSParser crashes when it encounters a version macro
	// without this.
	[parser setGenerateStandards: YES];

	// Generate a plist representing the class
	NSDictionary *module = 
		[parser parseFile: header
		         isSource: NO];

	NSDictionary *classes = [module objectForKey: @"Classes"];
	for (NSString *className in classes)
	{
		id pool = [NSAutoreleasePool new];
		NSMutableArray *mappings = [NSMutableArray array];

		// Substitute CF for NS in the new type
		// TODO: Allow this to be overridden in types.plist
		NSString *typeName = classToType(className);
		// Get the C type thing.
		NSString *cTypeName = 
			[typeMappings objectOrKey: [className stringByAppendingString: @"*"]];
		NSDictionary *class = [classes objectForKey: className];
		NSDictionary *methods = [class objectForKey: @"Methods"];
		for (NSString *methodName in methods)
		{
			NSMutableDictionary *function = [NSMutableDictionary dictionary];
			NSMutableString *functionName = [typeName mutableCopy];
			NSDictionary *method = [methods objectForKey: methodName];
			if ([methodName characterAtIndex: 0] == '+')
			{
				[function setObject: [NSNumber numberWithBool: YES]
                             forKey: @"ClassFunction"];
			}
			NSArray *selectorComponents = [method objectForKey: @"Sels"];
			NSArray *argTypes = [method objectForKey: @"Types"];
			NSMutableArray *functionArgTypes = [NSMutableArray array];
			NSUInteger args = [selectorComponents count];

			NSMutableArray *argMappings = [NSMutableArray array];
			int hiddenArgs = 0;
			id argNames = [method objectForKey: @"Args"];

			BOOL isClassMethod = [methodName characterAtIndex: 0] == '+';
			// For instance methods, add a self parameter
			if (!isClassMethod)
			{
				[functionArgTypes insertObject: cTypeName atIndex: 0];
				if (nil != argNames)
				{
					argNames = [argNames mutableCopy];
					[argNames insertObject: @"self" atIndex: 0];
				}
				else
				{
					argNames = [NSArray arrayWithObject: @"self"];
				}
				hiddenArgs = 1;
			}

			for (NSUInteger i=0 ; i<args ; i++)
			{
				NSString *type = [argTypes objectAtIndex: i];
				id functionNameComponent =
					[[selectorComponents objectAtIndex: i] mutableCopy];
				// Make the selector component start with a capital
				unichar start = [functionNameComponent characterAtIndex: 0];
				start = toupper(start);
				NSString *firstChar = [NSString stringWithCharacters: &start length: 1];
				[functionNameComponent replaceCharactersInRange: NSMakeRange(0,1)
													 withString: firstChar];
				// Trim the trailing colon
				if (nil != type)
				{
					functionNameComponent = 
						[functionNameComponent substringToIndex:
							[functionNameComponent length] - 1];
					type = [typeMappings objectOrKey: type];
					[functionArgTypes addObject: type];
				}
				[argMappings addObject: [NSNumber numberWithInteger: i + hiddenArgs]];
				[functionName appendString: functionNameComponent];
			}


			if ([functionArgTypes count] == 0)
			{
				[functionArgTypes addObject: @"void"];
			}

			id methodReturnType = [method objectForKey: @"ReturnType"];
			id functionReturnType = [typeMappings objectOrKey: methodReturnType];

			[mappings addObject: [NSDictionary dictionaryWithObjectsAndKeys:
				functionName, @"FunctionName",
				methodName, @"MethodName",
				functionArgTypes, @"FunctionArgTypes",
				methodReturnType, @"MethodReturnType",
				functionReturnType, @"FunctionReturnType",
				[NSNumber numberWithBool: isClassMethod], @"ClassMethod",
				selectorComponents, @"Selector",
				argMappings, @"ArgMappings",
				argNames, @"ArgNames",
				argTypes, @"MethodArgTypes",
				nil]];
		}
		[[NSDictionary dictionaryWithObjectsAndKeys:
			mappings, @"Mappings",
			cTypeName, @"CType",
			className, @"ClassName",
			nil] writeToFile: [className stringByAppendingPathExtension: @"plist"]
		          atomically: NO];
		[pool release];
	}
	return [classes allKeys];
}

void generateSourceForClass(NSString *classFile, BOOL generateHeader)
{
	NSString *plistFile = [classFile stringByAppendingPathExtension: @"plist"];
	NSDictionary *class =
		[NSDictionary dictionaryWithContentsOfFile: plistFile];
	NSArray *mappings = [class objectForKey: @"Mappings"];
	NSString *cTypeName = [class objectForKey: @"CType"];
	NSString *className = [class objectForKey: @"ClassName"];
	NSFileHandle *sourceFile = [NSFileHandle fileHandleForWritingNewFileAtPath: 
						[classToType(className) stringByAppendingPathExtension: @"m"]];
	NSFileHandle *headerFile = nil;
	if (generateHeader)
	{
		headerFile = [NSFileHandle fileHandleForWritingNewFileAtPath:
						[classToType(className) stringByAppendingPathExtension: @"h"]];
		[headerFile writeFormat: @"#ifdef __OBJC__\n"];
		[headerFile writeFormat: @"@class %@;\n", className];
		[headerFile writeFormat: @"typedef %@* %@;\n", className, cTypeName];
		[headerFile writeFormat: @"#else\n"];
		[headerFile writeFormat: @"typedef struct %@* %@;\n", className,
			cTypeName];
		[headerFile writeFormat: @"#endif // __OBJC__\n\n"];
	}

	for (NSDictionary *mapping in mappings)
	{
		NSString *functionReturn = [mapping objectForKey: @"FunctionReturnType"];
		NSString *functionName = [mapping objectForKey: @"FunctionName"];
		NSArray *argTypes = [mapping objectForKey: @"FunctionArgTypes"];
		NSArray *methodArgTypes = [mapping objectForKey: @"MethodArgTypes"];
		NSArray *argNames = [mapping objectForKey: @"ArgNames"];
		NSArray *selector = [mapping objectForKey: @"Selector"];
		NSArray *argMappings = [mapping objectForKey: @"ArgMappings"];
		NSUInteger argCount = [argTypes count];

		BOOL isClassMethod = [[mapping objectForKey: @"ClassMethod"] boolValue];

		NSMutableString *function = [NSMutableString stringWithFormat: @"%@ %@(%@",
				functionReturn, functionName, [argTypes objectAtIndex: 0]];
		// This can't be part of the format in case the type is void (then
		// no arg name)
		if (argCount > 0)
		{
			[function appendFormat: @" %@", [argNames objectAtIndex: 0]];
		}
		// Add the other names
		for (NSUInteger i=1 ; i<argCount ; i++)
		{
			[function appendFormat: @", %@ %@", [argTypes objectAtIndex: i],
				[argNames objectAtIndex: i]];
		}
		[function appendString: @")"];
		[headerFile writeFormat: @"%@;\n", function];
		[function appendString: @"\n{\n\t"];
		if (![@"void" isEqualToString: functionReturn])
		{
			[function appendString: @"return "];
		}
		[function appendString: @"["];

		if (isClassMethod)
		{
			[function appendString: className];
		}
		else
		{
			[function appendFormat: @"(%@*)self", className];
		}

		if ([methodArgTypes count] == 0)
		{
			[function appendFormat: @" %@", [selector objectAtIndex: 0]];
		}
		else
		{
			for (NSUInteger i=0 ; i<[selector count] ; i++)
			{
				[function appendFormat: @" %@ (%@)%@",
					[selector objectAtIndex: i],
					[methodArgTypes objectAtIndex: i],
					[argNames objectAtIndex: 
						[[argMappings objectAtIndex: i] intValue]]];
			}
		}

		[function appendString: @"];\n}\n\n"];
		[sourceFile writeFormat: @"%@", function];
	}
}

int main(void)
{
	[NSAutoreleasePool new];
	// Load the plist containing the ObjC->C type mappings.
	typeMappings = 
		[NSDictionary dictionaryWithContentsOfFile: @"types.plist"];
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	// Parse headers and emit plists for them
	id headers = [defaults stringForKey: @"headers"];
	if ([headers isKindOfClass: [NSString class]])
	{
		parseHeader(headers);
	}
	if ([headers isKindOfClass: [NSArray class]])
	{
		for (NSString *header in headers)
		{
			parseHeader(header);
		}
	}
	// Parse plists and generate headers
	BOOL generateHeaders = 
		[[defaults stringForKey: @"generateHeaders"] boolValue];
	id classes = [defaults stringForKey: @"classes"];
	if ([classes isKindOfClass: [NSString class]])
	{
		generateSourceForClass(classes, generateHeaders);
	}
	if ([classes isKindOfClass: [NSArray class]])
	{
		for (NSString *class in classes)
		{
			generateSourceForClass(class, generateHeaders);
		}
	}
	return 0;
}
