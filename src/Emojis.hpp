struct Emojis {
  enum struct Type {
    OkayWithThreeVSigns,
    WeightScale,
  };
  static const char* get(Type);
};
