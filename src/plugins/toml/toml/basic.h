// clang-format off
ksNew (16,
	keyNew (PREFIX "/a",
		KEY_VALUE, "3",
		KEY_META, "type", "long_long",
		KEY_META, "order", "0",
	KEY_END),
	keyNew (PREFIX "/a/b/c/d",
		KEY_VALUE, "5",
		KEY_META, "type", "long_long",
		KEY_META, "order", "1",
	KEY_END),
	keyNew (PREFIX "/b",
		KEY_VALUE, "hello",
		KEY_META, "tomltype", "string_basic",
		KEY_META, "type", "string",
		KEY_META, "order", "2",
	KEY_END),
	keyNew (PREFIX "/c",
		KEY_VALUE, "hello2",
		KEY_META, "tomltype", "string_literal",
		KEY_META, "type", "string",
		KEY_META, "order", "3",
	KEY_END),
	keyNew (PREFIX "/d",
		KEY_VALUE, "3.1415",
		KEY_META, "type", "double",
		KEY_META, "order", "4",
	KEY_END),
	keyNew (PREFIX "/e",
		KEY_VALUE, "+inf",
		KEY_META, "type", "double",
		KEY_META, "order", "5",
	KEY_END),
	keyNew (PREFIX "/f",
		KEY_VALUE, "-nan",
		KEY_META, "type", "double",
		KEY_META, "order", "6",
	KEY_END),
	keyNew (PREFIX "/g",
		KEY_VALUE, "1.003E-05",
		KEY_META, "type", "double",
		KEY_META, "order", "7",
	KEY_END),
	keyNew (PREFIX "/3/14",
		KEY_VALUE, "PI",
		KEY_META, "tomltype", "string_basic",
		KEY_META, "type", "string",
		KEY_META, "order", "8",
	KEY_END),
KS_END)
