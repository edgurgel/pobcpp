template<typename Type>
void func2() {
	Type::func();
}

class Etc {
	static void func() {

	}
};

class HelloWorld {
	public:
	unit World;
	unit Nothing;
	unit Hello {
		public:
		Hello();
	//	void sendHello();
		void sendHello() {
			comm->send<HelloWorld::World>(42, 1);
			int data = 45;
			comm->broadcast<Hello>(data);
		}

	};

	unit World {
		public:
		World();
		int recvHello();

	};
	unit Nothing [i:n] {
		public:
		Nothing(unsigned int i, unsigned int n);
		void doNothing();
	};
	static int func() { }	

//public: static Pobcpp::Pob_Type_Array __get_types() { Pobcpp::Pob_Type_Array pobtypes(3); pobtypes.add_type<Hello>(0);pobtypes.add_type<World>(1);pobtypes.add_type<Nothing>(2, 1);return pobtypes; } private:}; //};
	};
HelloWorld::Hello::Hello() {
	create_unit<class HelloWorld, class HelloWorld::Hello>(this);
}
