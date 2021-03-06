getTypeDefs =
function(namespace)
{
  typeClass= getClass("Type", .Force=TRUE, where=namespace)
   types = .Call("R_getTypeDefinitions", typeClass)

   names(types) = c("Void", "Label", "Float", "Double", "Int1", "Int8", "Int16", 
                     "Int32", "Int64", "FloatPtr", "DoublePtr", "Int32Ptr", "String")
   types
}

.onLoad =
function(...)
{
  
  e = getNamespace("Rllvm")

  types = getTypeDefs(e)

  mapply(utils::assignInNamespace,
         paste(names(types), "Type", sep = ""),
         types, 
         MoreArgs = list(ns = e))

# The StructType class isn't available yet at this stage in loading the package.
  tmp =  makeSEXPTypes(rawPointer = TRUE)
  mapply(utils::assignInNamespace,
          c("SEXPType", sprintf("%sSXPType", names(tmp)[-1])),
          tmp, MoreArgs = list(ns = e))
  
  if(getOption("Rllvm.autoInitialize", TRUE)) {
     InitializeNativeTarget()
		.Call("R_Pass_Initialize")
	}
}
