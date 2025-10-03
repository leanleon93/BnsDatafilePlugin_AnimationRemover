namespace Data {
	class SkillTraitJobStyleHelper {
	public:
		static int GetSpecIndexForJobStyleType(__int8 jobStyleType) {
			switch (jobStyleType)
			{
			case 0:
				return 1;
			case 1:
				return 2;
			case 2:
				return 3;
			case 3:
				return 4;
			case 4:
				return 5;
			case 5:
				return 1;
			case 6:
				return 2;
			case 7:
				return 3;
			case 8:
				return 4;
			case 9:
				return 5;
			case 10:
				return 0;
			default:
				return 0;
			}
		}
	};
}